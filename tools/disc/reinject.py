#!/usr/bin/env python3
"""reinject.py - write a rebuilt .T (flat 2048-B/sector image) back into a PSX
disc .bin, in place, at a file's sector range.

Target: SLUS-01323 (v1.1).  Companion to tools/extract/t_repack.py (which builds
the flat .T) and docs/AUTHORING.md.  Generalizes the FDAT-only `reinsert()` from
tools/farsi/build_rtl_patch.py to ANY file in disc_map/disc_files.json.

The flat .T is the de-sectored payload (DATA_SIZE bytes per sector).  Each sector
N of it is written to the user-data window of disc sector (sector_first + N),
i.e. byte (sector_first+N)*SECTOR_RAW + DATA_OFFSET.  This is an IN-PLACE patch:
the rebuilt file must not span MORE sectors than the original (t_repack preserves
size for in-place edits, so this holds).  For size growth, rebuild the image with
psxinject instead (see docs/AUTHORING.md).

Usage:
  tools/disc/reinject.py --file GG/COM/FDAT.T --t fdat_patched.T \
      --out "/path/Armored Core (v1.1) [TEST].bin"
  # --in defaults to filemap source_bin; a .cue is written next to --out.
"""
import argparse, json, os, shutil, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FILEMAP = os.path.join(ROOT, "disc_map", "disc_files.json")

SECTOR_RAW  = 2352
DATA_OFFSET = 24            # 12 sync + 4 header + 8 subheader (mode2/form1)
DATA_SIZE   = 2048          # user data per sector


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


def reinject(in_bin, out_bin, sector_first, sector_last, flat_t, write_cue=True):
    """Copy in_bin -> out_bin and overwrite the file's sectors with flat_t."""
    span = sector_last - sector_first + 1
    if len(flat_t) % DATA_SIZE != 0:
        sys.exit(f"flat .T length {len(flat_t)} not a multiple of {DATA_SIZE}")
    nsec = len(flat_t) // DATA_SIZE
    if nsec > span:
        sys.exit(f"flat .T is {nsec} sectors but file span is only {span} "
                 f"(size growth needs psxinject / image rebuild)")
    if os.path.abspath(in_bin) != os.path.abspath(out_bin):
        shutil.copyfile(in_bin, out_bin)
    with open(out_bin, "r+b") as b:
        for i in range(nsec):
            b.seek((sector_first + i) * SECTOR_RAW + DATA_OFFSET)
            b.write(flat_t[i * DATA_SIZE:(i + 1) * DATA_SIZE])
    if write_cue:
        cue = os.path.splitext(out_bin)[0] + ".cue"
        with open(cue, "w") as c:
            c.write(f'FILE "{os.path.basename(out_bin)}" BINARY\n'
                    f'  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
    return nsec, span


def write_cue(out_bin):
    cue = os.path.splitext(out_bin)[0] + ".cue"
    with open(cue, "w") as c:
        c.write(f'FILE "{os.path.basename(out_bin)}" BINARY\n'
                f'  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
    return cue


def reinject_grow(in_bin, out_bin, iso_path, flat_t, psxinject="psxinject"):
    """Rebuild the image with a (possibly larger) file via psxinject.

    psxinject edits the image IN PLACE and needs a matching .cue, so we copy
    in_bin -> out_bin, write out_bin's .cue, drop the new payload to a temp file,
    and replace `iso_path` inside out_bin.  Use this when the rebuilt .T grows past
    the original sector span (the in-place `reinject()` cannot).
    """
    if os.path.abspath(in_bin) != os.path.abspath(out_bin):
        shutil.copyfile(in_bin, out_bin)
    write_cue(out_bin)
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tf:
        tf.write(flat_t)
        payload = tf.name
    try:
        r = subprocess.run([psxinject, out_bin, iso_path, payload],
                           capture_output=True, text=True)
        if r.returncode != 0 or "replaced" not in (r.stdout + r.stderr):
            sys.exit(f"psxinject failed:\n{r.stdout}\n{r.stderr}")
    finally:
        os.unlink(payload)
    return r.stdout.strip().splitlines()[-1] if r.stdout else "ok"


def main():
    ap = argparse.ArgumentParser(description="reinject a flat .T into a PSX disc .bin")
    ap.add_argument("--file", required=True, help="file id/path from disc_files.json")
    ap.add_argument("--t", required=True, help="rebuilt flat .T (2048-B/sector)")
    ap.add_argument("--in", dest="in_bin", help="source .bin (default: filemap source_bin)")
    ap.add_argument("--out", required=True, help="output patched .bin")
    ap.add_argument("--no-cue", action="store_true", help="do not write a .cue")
    ap.add_argument("--grow", action="store_true",
                    help="force psxinject rebuild (use when the .T grows past its "
                         "sector span; auto-selected if it does)")
    args = ap.parse_args()

    fm = load_filemap()
    in_bin = args.in_bin or fm["source_bin"]
    rec = find_file(fm, args.file)
    if not rec:
        sys.exit(f"file not in filemap: {args.file}")
    flat = open(args.t, "rb").read()
    span = rec["sector_last"] - rec["sector_first"] + 1
    nsec = -(-len(flat) // DATA_SIZE)  # ceil
    iso_path = rec.get("id") or rec.get("path")
    if args.grow or nsec > span:
        why = "forced --grow" if args.grow else f"grows {nsec}>{span} sectors"
        last = reinject_grow(in_bin, args.out, iso_path, flat)
        print(f"reinject (psxinject, {why}) {rec['path']} into {args.out}: {last}")
    else:
        nsec, span = reinject(in_bin, args.out, rec["sector_first"],
                              rec["sector_last"], flat, write_cue=not args.no_cue)
        print(f"reinjected {nsec}/{span} sectors of {rec['path']} "
              f"@ sector {rec['sector_first']} into {args.out}")


if __name__ == "__main__":
    main()
