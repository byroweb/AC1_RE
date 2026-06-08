#!/usr/bin/env python3
"""
build_filemap.py — parse a jPSXdec .idx into disc_map/disc_files.json.

The jPSXdec index is the authoritative file map of the AC1 (SLUS-01323 v1.1)
disc: every file with exact sector range + size, plus introspected sub-streams
(Tim / XA / Video). We reuse it instead of writing an ISO9660 parser.

Output JSON schema:
{
  "source_bin": "...",
  "sector_size": 2352, "sector_count": N, "first_sector_offset": 0,
  "files": [ { "idx": int, "id": str, "path": str,
               "sector_first": int, "sector_last": int, "size": int,
               "mode2form2": bool, "cd_audio": bool,
               "children": [ {"idx","id","type","sector_first","sector_last", ...} ] } ]
}

Usage: python3 tools/build_filemap.py [AC_1_USA_test.idx] [disc_map/disc_files.json]
"""
import sys, os, json, re

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IDX_DEFAULT = os.path.join(HERE, "AC_1_USA_test.idx")
OUT_DEFAULT = os.path.join(HERE, "disc_map", "disc_files.json")

# The .idx was built against a *patched* work image ("AC_1_USA_test"). For a clean
# text inventory we must read the PRISTINE original. Sector layout is identical
# (same ISO/size), so the .idx sector ranges remain valid against the pristine bin.
PRISTINE_BIN = "/home/byron/Desktop/Armored_Core_Hacks/AC_1_USA_backup/Armored Core (v1.1).bin"


def parse_kv(line):
    """jPSXdec rows are pipe-delimited key:value (ID/Path may contain ':' and '/')."""
    d = {}
    for field in line.rstrip("\n").split("|"):
        if ":" in field:
            k, _, v = field.partition(":")
            d[k.strip()] = v.strip()
    return d


def parse_sectors(s):
    m = re.match(r"(\d+)-(\d+)", s or "")
    if not m:
        return (None, None)
    return (int(m.group(1)), int(m.group(2)))


def main():
    idx_path = sys.argv[1] if len(sys.argv) > 1 else IDX_DEFAULT
    out_path = sys.argv[2] if len(sys.argv) > 2 else OUT_DEFAULT

    header = {}
    files = []          # parent File records, in order
    by_id = {}          # base id -> file record (for attaching children)

    with open(idx_path, "r", errors="replace") as f:
        for raw in f:
            if raw.startswith(";") or raw.startswith("["):
                continue
            if raw.startswith("Filename:"):
                kv = parse_kv(raw)
                header = {
                    "source_bin": kv.get("Filename"),
                    "sector_size": int(kv.get("Sector size", 2352)),
                    "sector_count": int(kv.get("Sector count", 0)),
                    "first_sector_offset": int(kv.get("First sector offset", 0)),
                }
                continue
            if not raw.startswith("#:"):
                continue

            kv = parse_kv(raw)
            ident = kv.get("ID", "")
            typ = kv.get("Type", "")
            first, last = parse_sectors(kv.get("Sectors"))
            rec_idx = int(kv.get("#", -1))

            if typ == "File":
                rec = {
                    "idx": rec_idx,
                    "id": ident,
                    "path": kv.get("Path", ident),
                    "sector_first": first,
                    "sector_last": last,
                    "size": int(kv.get("Size", 0)),
                    "mode2form2": kv.get("Has mode 2 form 2", "No") == "Yes",
                    "cd_audio": kv.get("Has CD audio", "No") == "Yes",
                    "children": [],
                }
                files.append(rec)
                by_id[ident] = rec
            else:
                # sub-stream: ID looks like "GG/MS/MIS.T[3]" or "GG/BGM/BGM00.XA[0]"
                base = re.sub(r"\[\d+\]$", "", ident)
                child = {
                    "idx": rec_idx,
                    "id": ident,
                    "type": typ,
                    "sector_first": first,
                    "sector_last": last,
                }
                # carry a few useful introspected fields when present
                for opt in ("Dimensions", "Bpp", "Palettes", "Start Offset",
                            "Channel", "Stereo?", "Samples/Sec", "Sector stride"):
                    if opt in kv:
                        child[opt] = kv[opt]
                parent = by_id.get(base)
                if parent is not None:
                    parent["children"].append(child)
                else:
                    # orphan (shouldn't happen) — keep as standalone
                    files.append({"idx": rec_idx, "id": ident, "type": typ,
                                  "sector_first": first, "sector_last": last,
                                  "orphan": True})

    # Override source to the pristine image for clean reads (see PRISTINE_BIN note).
    if os.path.exists(PRISTINE_BIN):
        header["idx_source_bin"] = header.get("source_bin")
        header["source_bin"] = PRISTINE_BIN

    out = dict(header)
    out["file_count"] = len(files)
    out["files"] = files
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w") as f:
        json.dump(out, f, indent=1)

    # ---- summary + validation against REFERENCE.md §9 ground truth ----
    def find(p):
        return next((r for r in files if r.get("path") == p or r.get("id") == p), None)

    print(f"Parsed {len(files)} files from {os.path.basename(idx_path)} -> {out_path}")
    checks = {
        "GG/COM/FDAT.T": (72189, 85330),
        "GG/MS/MIS.T":   (101920, 103449),
        "GG/P0/PA00.T":  (103637, 104066),
        "SLUS_013.23":   (39, 121),
    }
    ok = True
    for path, (ef, el) in checks.items():
        r = find(path)
        got = (r["sector_first"], r["sector_last"]) if r else None
        good = got == (ef, el)
        ok = ok and good
        print(f"  {'OK ' if good else 'BAD'} {path}: {got} (expect {(ef, el)})")

    pa = [r for r in files if re.search(r"/PA\d{2}\.T$", r.get("path", ""))]
    mis = find("GG/MS/MIS.T")
    print(f"  PA##.T base files: {len(pa)}  (range {pa[0]['path']}..{pa[-1]['path']})")
    print(f"  MIS.T children: {len(mis['children']) if mis else 0} "
          f"({sum(1 for c in mis['children'] if c.get('type')=='Tim') if mis else 0} Tim)")
    print("VALIDATION:", "PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
