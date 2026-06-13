#!/usr/bin/env python3
"""
t_repack.py — AC1 ".T" container WRITER / repacker (inverse of extract_t.py).

A ".T" archive is a sector-based container (REFERENCE.md §3, MXT_LOADER.md):
  - Sector 0 (2048 B) = TOC: a packed uint16 array of per-entry start-sectors.
      * count-first  : u16[0] = entry COUNT, then count+1 start-sectors (FDAT, PA,
        MIS, every AC1 .T seen). Entry i = flat[off[i]*2048 : off[i+1]*2048].
      * offset-first : u16[0] = first entry's start sector (==1); auto-detected.
  - Entries are sector-aligned (2048 B/sector); zero-length entries (repeated
    offsets) are common and preserved.
  - Every nonempty entry ends with a trailing uint32 CHECKSUM word
    (seed 0x12345678 + sum of all earlier words), game routine FUN_80015b24.
    Ground truth on the pristine disc: FDAT 123/123, PA00 114/114, MIS 546/547
    nonempty entries carry a valid checksum (one MIS text entry's final word is
    legitimately payload). Any EDITED entry must have its last word recomputed or
    the game hangs at "NOW LOADING".

This module operates on the FLAT, de-sectored image (2048 B/sector) — the same
bytes extract_t.py produces. Raw 2352-byte disc-sector reframing + reinjection
(jPSXdec/psxinject) is a separate downstream step; see docs/T_REPACK.md.

The hard gate (docs/AC1MOD_VISION.md): pack(unpack(flat)) is byte-identical to
flat for every real container. pack() REBUILDS the TOC from entry sizes (it does
not echo the original TOC bytes), so a green round-trip proves the writer math.

CLI:
  python3 tools/t_repack.py --file GG/COM/FDAT.T --verify-roundtrip
  python3 tools/t_repack.py --file GG/MS/MIS.T  --check-checksums
  python3 tools/t_repack.py --file GG/COM/FDAT.T --replace-entry 4 new.bin -o out.T
"""
import sys, os, json, struct, argparse

HERE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FILEMAP = os.path.join(HERE, "disc_map", "disc_files.json")

# Mode-2/Form-1 sector math, shared with extract_t.py.
SECTOR_RAW  = 2352
DATA_OFFSET = 24            # 12 sync + 4 header + 8 subheader
DATA_SIZE   = 2048          # user data per sector
USHORTS     = DATA_SIZE // 2

CHECKSUM_SEED = 0x12345678  # FUN_80015b24 (re/ac1_mxt: AC1_MXT_CHECKSUM_SEED)


# ---------------------------------------------------------------------------
# Checksum (Python port of re/src/ac1_mxt.c — that C model stays the authority;
# tools/t_repack_test.py asserts this port agrees with it byte-for-byte).
# ---------------------------------------------------------------------------
def checksum(entry):
    """seed + sum of every 32-bit word EXCEPT the last. `entry` len % 4 == 0."""
    nwords = len(entry) // 4
    s = CHECKSUM_SEED
    if nwords == 0:
        return s
    for i in range(nwords - 1):
        s = (s + struct.unpack_from("<I", entry, i * 4)[0]) & 0xFFFFFFFF
    return s


def verify(entry):
    """True if the entry's stored trailing word matches its computed checksum."""
    if len(entry) < 4:
        return False
    stored = struct.unpack_from("<I", entry, len(entry) - 4)[0]
    return checksum(entry) == stored


def fix_checksum(entry):
    """Return `entry` with its trailing word rewritten so verify() passes."""
    if len(entry) < 4:
        return bytes(entry)
    b = bytearray(entry)
    struct.pack_into("<I", b, len(b) - 4, checksum(b))
    return bytes(b)


# ---------------------------------------------------------------------------
# Container model
# ---------------------------------------------------------------------------
class Container:
    """A parsed flat .T image: the entries plus enough TOC metadata to rebuild
    the original sector 0 byte-for-byte."""
    __slots__ = ("convention", "entries", "terminator", "terminator_is_total",
                 "toc_extra", "total_sectors")

    def __init__(self, convention, entries, terminator, terminator_is_total,
                 toc_extra, total_sectors):
        self.convention = convention            # "count-first" | "offset-first"
        self.entries = entries                  # list[bytes], sector-aligned, 0-len ok
        self.terminator = terminator            # raw final offset word
        self.terminator_is_total = terminator_is_total  # was it == total sectors?
        self.toc_extra = toc_extra              # sector-0 bytes after the u16 table
        self.total_sectors = total_sectors


def unpack(flat):
    """Parse a flat .T image into a Container."""
    if len(flat) % DATA_SIZE != 0 or len(flat) < DATA_SIZE:
        raise ValueError(f"flat image not a sector multiple: {len(flat)} B")
    total = len(flat) // DATA_SIZE
    u = struct.unpack_from(f"<{USHORTS}H", flat, 0)

    # Convention detection (mirrors extract_t.parse_toc): count-first if u[0] is an
    # entry count (u[1]==1 starts the offset run and u[0] > u[1]); else offset-first.
    count_first = (u[1] == 1 and u[0] > u[1] and u[0] <= total * 2)
    if count_first:
        n = u[0]
        offsets = list(u[1:1 + n + 1])
        used_u16 = 1 + (n + 1)
        convention = "count-first"
    else:
        # offset-first: leading non-decreasing run of plausible sector offsets.
        offsets, prev = [], -1
        for v in u:
            if v < prev or v > total or v == 0xFFFF:
                break
            offsets.append(v); prev = v
        n = len(offsets) - 1
        used_u16 = len(offsets)
        convention = "offset-first"

    entries = []
    for i in range(n):
        a, b = offsets[i], offsets[i + 1]
        # The reader rule (pa_obj.load_container): a non-increasing pair yields an
        # empty entry — this is how zero-length entries AND the MIS terminator
        # (final offset 0) decode.
        entries.append(flat[a * DATA_SIZE: b * DATA_SIZE] if b > a else b"")

    terminator = offsets[-1]
    return Container(
        convention=convention,
        entries=entries,
        terminator=terminator,
        terminator_is_total=(terminator == total),
        toc_extra=flat[used_u16 * 2: DATA_SIZE],
        total_sectors=total,
    )


def _sectors(n_bytes):
    return (n_bytes + DATA_SIZE - 1) // DATA_SIZE


def rebuild_offsets(c):
    """Reconstruct the start-sector table from entry sizes + the captured
    terminator. Interior offsets are exact (each entry occupies exactly its
    sector count); only the final word follows the terminator rule."""
    offs = [1] if c.convention == "count-first" else [1]
    for e in c.entries:
        offs.append(offs[-1] + _sectors(len(e)))
    # Final word: a real end-sentinel (== total sectors) tracks the new total; a
    # literal terminator (e.g. MIS's 0) is preserved verbatim.
    if c.terminator_is_total:
        offs[-1] = offs[-1]            # cumulative end == new total
    else:
        offs[-1] = c.terminator
    return offs


def pack(c):
    """Rebuild the flat .T image from a Container (byte-identical for unedited)."""
    offs = rebuild_offsets(c)
    n = len(c.entries)

    toc = bytearray(DATA_SIZE)
    if c.convention == "count-first":
        struct.pack_into("<H", toc, 0, n)
        struct.pack_into(f"<{n + 1}H", toc, 2, *offs)
        used_u16 = 1 + (n + 1)
    else:
        struct.pack_into(f"<{len(offs)}H", toc, 0, *offs)
        used_u16 = len(offs)
    # Restore verbatim tail bytes (verified all-zero on the disc, but preserved
    # in case any container carries nonzero padding).
    toc[used_u16 * 2: DATA_SIZE] = c.toc_extra[: DATA_SIZE - used_u16 * 2]

    out = bytearray(toc)
    for e in c.entries:
        out += e
        pad = (-len(e)) % DATA_SIZE
        if pad:
            out += b"\x00" * pad
    return bytes(out)


def replace_entry(c, i, payload, do_fix_checksum=True):
    """Swap entry i for `payload`, sector-pad it, and recompute its checksum."""
    if not (0 <= i < len(c.entries)):
        raise IndexError(f"entry {i} out of range (0..{len(c.entries)-1})")
    b = bytearray(payload)
    pad = (-len(b)) % DATA_SIZE
    b += b"\x00" * pad
    e = fix_checksum(b) if do_fix_checksum else bytes(b)
    c.entries[i] = e
    # A grown/shrunk entry shifts later offsets; if the terminator was the true
    # end sentinel it will track the new total automatically in rebuild_offsets.
    if c.terminator_is_total:
        c.total_sectors = 1 + sum(_sectors(len(x)) for x in c.entries)
    return c


# ---------------------------------------------------------------------------
# Disc / filemap helpers (shared shape with extract_t.py)
# ---------------------------------------------------------------------------
def load_filemap():
    with open(FILEMAP) as f:
        return json.load(f)


def find_file(fm, key):
    for r in fm["files"]:
        if r.get("path") == key or r.get("id") == key:
            return r
    for r in fm["files"]:
        p = r.get("path", "")
        if p.endswith("/" + key) or p.endswith("/" + key + ".T"):
            return r
    return None


def read_flat_from_disc(fm, rec):
    """De-sector a file out of the source .bin into a flat 2048-B/sector image."""
    first, last = rec["sector_first"], rec["sector_last"]
    out = bytearray()
    with open(fm["source_bin"], "rb") as f:
        for s in range(first, last + 1):
            f.seek(s * SECTOR_RAW + DATA_OFFSET)
            d = f.read(DATA_SIZE)
            if len(d) != DATA_SIZE:
                raise RuntimeError(f"short read at sector {s}")
            out += d
    return bytes(out)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description="AC1 .T container repacker")
    ap.add_argument("--file", required=True, help="file id/path from disc_files.json")
    ap.add_argument("--bin", help="override source .bin (default: from filemap)")
    ap.add_argument("--verify-roundtrip", action="store_true",
                    help="unpack -> pack -> compare vs source; nonzero exit on mismatch")
    ap.add_argument("--check-checksums", action="store_true",
                    help="report stored-vs-computed checksum per nonempty entry")
    ap.add_argument("--replace-entry", nargs=2, metavar=("N", "PAYLOAD"),
                    help="replace entry N with PAYLOAD bytes")
    ap.add_argument("--no-fix-checksum", action="store_true",
                    help="with --replace-entry, do NOT recompute the trailing word")
    ap.add_argument("-o", "--out", help="output .T path (for --replace-entry)")
    args = ap.parse_args()

    fm = load_filemap()
    if args.bin:
        fm = dict(fm); fm["source_bin"] = args.bin
    rec = find_file(fm, args.file)
    if not rec:
        sys.exit(f"file not in filemap: {args.file}")

    flat = read_flat_from_disc(fm, rec)
    c = unpack(flat)
    print(f"[{args.file}] {len(flat)//DATA_SIZE} sectors, {len(c.entries)} entries, "
          f"{c.convention}, terminator={c.terminator}"
          f"{' (==total)' if c.terminator_is_total else ''}")

    if args.verify_roundtrip:
        re = pack(c)
        ok = (re == flat)
        print(f"  round-trip: {'OK byte-identical' if ok else 'MISMATCH'}")
        if not ok:
            for i in range(0, min(len(flat), len(re)), DATA_SIZE):
                if flat[i:i+DATA_SIZE] != re[i:i+DATA_SIZE]:
                    print(f"    first differing sector: {i//DATA_SIZE}")
                    break
            print(f"    lengths: src={len(flat)} repacked={len(re)}")
            return 1

    if args.check_checksums:
        ok = bad = 0
        for i, e in enumerate(c.entries):
            if len(e) == 0:
                continue
            if verify(e):
                ok += 1
            else:
                bad += 1
                stored = struct.unpack_from("<I", e, len(e) - 4)[0]
                print(f"    e{i}: checksum MISMATCH "
                      f"(stored {stored:#010x} != computed {checksum(e):#010x})")
        print(f"  checksums: {ok} OK, {bad} mismatch "
              f"of {ok + bad} nonempty")

    if args.replace_entry:
        n = int(args.replace_entry[0])
        with open(args.replace_entry[1], "rb") as f:
            payload = f.read()
        replace_entry(c, n, payload, do_fix_checksum=not args.no_fix_checksum)
        out = pack(c)
        dest = args.out or (os.path.basename(rec["path"]).replace(".", "_") + "_repacked.T")
        with open(dest, "wb") as f:
            f.write(out)
        print(f"  replaced entry {n} ({len(payload)} B payload), wrote {dest} "
              f"({len(out)//DATA_SIZE} sectors)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
