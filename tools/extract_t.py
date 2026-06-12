#!/usr/bin/env python3
"""
extract_t.py — generalized AC1 ".T" container extractor.

A ".T" file is a sector-based container (docs/REFERENCE.md §3):
  - Sector 0 (2048 B): TOC = packed uint16 array of sector offsets.
    Entry i spans sectors TOC[i] .. TOC[i+1]-1.
  - TWO conventions for uint16[0]:
      * "offset-first": TOC[0] is the first entry's sector offset (== 1 normally).
      * "count-first" : TOC[0] is the ENTRY COUNT, real offsets start at TOC[1]
        (seen in MENU_TIM.T = 122, and PA00.T = 176). Auto-detected below.
  - Zero-length entries are common (repeated offsets) and tolerated.
  - Each entry carries a trailing uint32 checksum word (docs/REFERENCE.md §6) — we do
    NOT touch it here; extraction is read-only. Checksums only matter on re-inject.

Reuses the Mode-2/Form-1 sector math from extract_fdat.py:
  raw sector = 2352 B; user data at +24; 2048 B/sector.

CLI:
  # by file id/path (looked up in disc_map/disc_files.json):
  python3 tools/extract_t.py --file GG/MS/MIS.T
  python3 tools/extract_t.py --file GG/P0/PA00.T --dump-entries
  # by explicit sector range:
  python3 tools/extract_t.py --sectors 101920-103449
  # list TOC only, no payload extraction:
  python3 tools/extract_t.py --file GG/MS/MIS.T --toc-only

Outputs (default under disc_map/extracted/<name>/):
  <name>.T          flat 2048-B/sector container
  toc.json          parsed TOC: convention, entry count, per-entry sectors/bytes
  entries/NNN.bin   per-entry payloads (with --dump-entries)
"""
import sys, os, json, struct, argparse, re

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FILEMAP = os.path.join(HERE, "disc_map", "disc_files.json")

SECTOR_RAW  = 2352
DATA_OFFSET = 24          # Mode 2 Form 1: 12 sync + 4 header + 8 subheader
DATA_SIZE   = 2048
USHORTS     = DATA_SIZE // 2   # 1024 uint16 per sector


def load_filemap():
    with open(FILEMAP) as f:
        return json.load(f)


def find_file(fm, key):
    for r in fm["files"]:
        if r.get("path") == key or r.get("id") == key:
            return r
    # allow bare "PA00.T" / "PA00"
    for r in fm["files"]:
        p = r.get("path", "")
        if p.endswith("/" + key) or p.endswith("/" + key + ".T"):
            return r
    return None


def read_sectors(bin_path, first, count):
    """Return `count` user-data sectors (2048 B each) starting at LBA `first`."""
    out = bytearray()
    with open(bin_path, "rb") as f:
        for s in range(first, first + count):
            f.seek(s * SECTOR_RAW + DATA_OFFSET)
            d = f.read(DATA_SIZE)
            if len(d) != DATA_SIZE:
                raise RuntimeError(f"short read at sector {s}: {len(d)} B")
            out += d
    return bytes(out)


def parse_toc(sector0, total_sectors):
    """
    Parse the TOC sector. Returns (convention, offsets) where `offsets` is the
    list of entry start-sectors (ascending, monotonic non-decreasing), already
    normalized so entry i = offsets[i]..offsets[i+1]-1 using a trailing sentinel
    == total_sectors.
    """
    u = list(struct.unpack_from(f"<{USHORTS}H", sector0, 0))

    def monotonic_run(seq):
        """length of the leading non-decreasing run of plausible sector offsets."""
        n = 0
        prev = -1
        for v in seq:
            if v < prev or v > total_sectors or v == 0xFFFF:
                break
            prev = v
            n += 1
        return n

    # Heuristic: in offset-first files, u[0] is small (==1). In count-first files,
    # u[0] is the entry count and u[1] starts the offset table (u[1] typically 1).
    off_run = monotonic_run(u)
    cnt_run = monotonic_run(u[1:])

    # Decide convention. count-first signature: u[0] does NOT continue the run
    # that u[1:] forms, and u[1] == 1 (first entry starts at sector 1).
    count_first = (u[1] == 1 and (u[0] > u[1]) and (cnt_run >= off_run) and u[0] <= total_sectors * 2)
    if count_first:
        count = u[0]
        raw = u[1:1 + count]
    else:
        # offset-first: take the leading monotonic run as offsets
        raw = u[:off_run]
        count = len(raw)

    # Trim trailing padding/duplicate-sentinel beyond total_sectors and append
    # the real end sentinel so the last entry is bounded.
    offsets = []
    for v in raw:
        if v > total_sectors:
            break
        offsets.append(v)
    # number of *entries* is len(offsets) if the last value already equals
    # total_sectors (acts as sentinel); else we append total_sectors as sentinel.
    if not offsets:
        return ("empty", [])
    if offsets[-1] != total_sectors:
        offsets = offsets + [total_sectors]
    convention = "count-first" if count_first else "offset-first"
    return (convention, offsets)


def main():
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--file", help="file id/path from disc_files.json (e.g. GG/MS/MIS.T)")
    g.add_argument("--sectors", help="explicit LBA range first-last")
    ap.add_argument("--bin", help="override source .bin (default: from filemap)")
    ap.add_argument("--out", help="output dir (default disc_map/extracted/<name>)")
    ap.add_argument("--dump-entries", action="store_true", help="write entries/NNN.bin")
    ap.add_argument("--toc-only", action="store_true", help="parse TOC, no .T/payload write")
    ap.add_argument("--name", help="override output name")
    args = ap.parse_args()

    fm = load_filemap()
    bin_path = args.bin or fm["source_bin"]

    if args.file:
        rec = find_file(fm, args.file)
        if not rec:
            sys.exit(f"file not found in filemap: {args.file}")
        first, last = rec["sector_first"], rec["sector_last"]
        name = args.name or os.path.basename(rec["path"]).replace(".", "_")
    else:
        m = re.match(r"(\d+)-(\d+)$", args.sectors)
        if not m:
            sys.exit("--sectors must be FIRST-LAST")
        first, last = int(m.group(1)), int(m.group(2))
        rec = None
        name = args.name or f"sec_{first}_{last}"

    total_sectors = last - first + 1
    out_dir = args.out or os.path.join(HERE, "disc_map", "extracted", name)
    os.makedirs(out_dir, exist_ok=True)

    print(f"[{name}] sectors {first}-{last} ({total_sectors} sectors, "
          f"{total_sectors*DATA_SIZE:,} B) from {os.path.basename(bin_path)}")

    data = read_sectors(bin_path, first, total_sectors)

    if not args.toc_only:
        flat_path = os.path.join(out_dir, name + ".T")
        with open(flat_path, "wb") as f:
            f.write(data)
        print(f"  flat container -> {flat_path}")

    convention, offsets = parse_toc(data[:DATA_SIZE], total_sectors)
    n_entries = max(len(offsets) - 1, 0)
    print(f"  TOC convention: {convention}; {n_entries} entries")

    # Build per-entry table.
    entries = []
    tim_children = {}
    if rec:
        for c in rec.get("children", []):
            m = re.search(r"\[(\d+)\]$", c["id"])
            if m:
                tim_children[int(m.group(1))] = c

    for i in range(n_entries):
        s0, s1 = offsets[i], offsets[i + 1]
        nsec = s1 - s0
        byte_off = s0 * DATA_SIZE
        byte_len = nsec * DATA_SIZE
        e = {
            "index": i,
            "sector_start": s0,
            "sector_count": nsec,
            "byte_offset": byte_off,
            "byte_len": byte_len,
            "disc_lba_start": first + s0,
        }
        if nsec > 0:
            payload = data[byte_off:byte_off + byte_len]
            e["first8_hex"] = payload[:8].hex()
            # cheap classifier
            tag = "TIM" if payload[:4] == b"\x10\x00\x00\x00" else None
            if tag is None and payload[:4] == b"\x40\x00\x00\x00":
                tag = "TMD"
            if i in tim_children:
                tag = "TIM(idx)"
            e["guess"] = tag or "?"
        else:
            e["guess"] = "empty"
        entries.append(e)

        if args.dump_entries and nsec > 0:
            ed = os.path.join(out_dir, "entries")
            os.makedirs(ed, exist_ok=True)
            with open(os.path.join(ed, f"{i:03d}.bin"), "wb") as f:
                f.write(data[byte_off:byte_off + byte_len])

    toc = {
        "name": name,
        "disc_sector_first": first,
        "disc_sector_last": last,
        "total_sectors": total_sectors,
        "convention": convention,
        "n_entries": n_entries,
        "entries": entries,
    }
    toc_path = os.path.join(out_dir, "toc.json")
    with open(toc_path, "w") as f:
        json.dump(toc, f, indent=1)
    print(f"  TOC -> {toc_path}")

    # quick stats
    nonzero = [e for e in entries if e["sector_count"] > 0]
    guesses = {}
    for e in nonzero:
        guesses[e["guess"]] = guesses.get(e["guess"], 0) + 1
    print(f"  non-empty entries: {len(nonzero)}/{n_entries}  by guess: {guesses}")
    if nonzero:
        covered = offsets[-1] - offsets[0]
        print(f"  sector coverage: entries span {offsets[0]}..{offsets[-1]} "
              f"of {total_sectors} (sentinel-bounded)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
