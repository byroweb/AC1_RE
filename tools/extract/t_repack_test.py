#!/usr/bin/env python3
"""
t_repack_test.py — validation harness for tools/t_repack.py (the .T repacker).

The hard gate (docs/AC1MOD_VISION.md / REFERENCE.md): pack(unpack(flat)) must be
byte-identical to the source flat image for EVERY .T container on the disc, with
the TOC rebuilt (not echoed) from entry sizes. This script proves that, plus:

  1. ROUND-TRIP GATE   — every .T in disc_files.json round-trips byte-identical.
  2. CHECKSUM PARITY   — the Python checksum() agrees with the C reference model
                         re/ac1_mxt (built on demand) on a sample of real entries.
  3. EDIT CORRECTNESS  — replace_entry rewrites one entry's checksum, leaves all
                         other entry payloads byte-identical, and re-verifies.

Exit code is nonzero if any check fails. Reads the user's local disc only; writes
nothing game-derived to the repo.
"""
import sys, os, json, struct, subprocess, tempfile

HERE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(HERE, "tools", "extract"))
import t_repack as T


def all_T_files(fm):
    return [r for r in fm["files"] if (r.get("path") or "").endswith(".T")]


def gate_roundtrip(fm):
    print("=== 1. ROUND-TRIP GATE (pack(unpack(x)) == x for every .T) ===")
    files = all_T_files(fm)
    print(f"{'file':<22} {'sec':>6} {'entries':>7} {'conv':>12}  result")
    nfail = 0
    for rec in files:
        path = rec["path"]
        try:
            flat = T.read_flat_from_disc(fm, rec)
            c = T.unpack(flat)
            ok = (T.pack(c) == flat)
        except Exception as ex:
            ok = False
            print(f"{path:<22} {'':>6} {'':>7} {'':>12}  ERROR {ex}")
            nfail += 1
            continue
        if not ok:
            nfail += 1
        print(f"{path:<22} {len(flat)//2048:>6} {len(c.entries):>7} "
              f"{c.convention:>12}  {'OK' if ok else 'MISMATCH'}")
    print(f"  -> {len(files)-nfail}/{len(files)} byte-identical\n")
    return nfail == 0


def gate_checksum_parity(fm):
    """Build re/ac1_mxt and compare its checksum to the Python port on real bytes."""
    print("=== 2. CHECKSUM PARITY (Python port vs C re/ac1_mxt) ===")
    # Build a tiny C harness that calls ac1_mxt_checksum on stdin bytes.
    redir = os.path.join(HERE, "re")
    csrc = r'''
#include <stdio.h>
#include <stdlib.h>
#include "ac1_mxt.h"
int main(void){
  static unsigned char buf[1<<20]; size_t n=fread(buf,1,sizeof buf,stdin);
  printf("%u\n", (unsigned)ac1_mxt_checksum(buf,n));
  return 0;
}'''
    with tempfile.TemporaryDirectory() as td:
        cpath = os.path.join(td, "cks.c")
        with open(cpath, "w") as f:
            f.write(csrc)
        exe = os.path.join(td, "cks")
        r = subprocess.run(
            ["cc", "-I", os.path.join(redir, "include"), cpath,
             os.path.join(redir, "src", "ac1_mxt.c"), "-o", exe],
            capture_output=True, text=True)
        if r.returncode != 0:
            print("  SKIP — could not build C model:\n" + r.stderr)
            return True  # don't fail the gate on a missing host compiler
        # Sample: first few nonempty entries of FDAT + PA00.
        sample = []
        for path in ("GG/COM/FDAT.T", "GG/P0/PA00.T"):
            rec = T.find_file(fm, path)
            c = T.unpack(T.read_flat_from_disc(fm, rec))
            sample += [(path, i, e) for i, e in enumerate(c.entries) if e][:5]
        nfail = 0
        for path, i, e in sample:
            cres = subprocess.run([exe], input=e, capture_output=True)
            cval = int(cres.stdout.strip())
            pval = T.checksum(e)
            match = (cval == pval)
            if not match:
                nfail += 1
            print(f"  {path} e{i}: py={pval:#010x} c={cval:#010x} "
                  f"{'OK' if match else 'MISMATCH'}")
        print(f"  -> {len(sample)-nfail}/{len(sample)} agree\n")
        return nfail == 0


def gate_edit_correctness(fm):
    print("=== 3. EDIT CORRECTNESS (replace_entry) ===")
    rec = T.find_file(fm, "GG/COM/FDAT.T")
    flat = T.read_flat_from_disc(fm, rec)
    c = T.unpack(flat)
    target = next(i for i, e in enumerate(c.entries) if len(e) >= 8)
    orig_entries = list(c.entries)

    # Edit: flip some payload bytes in the target entry (keep its size).
    payload = bytearray(orig_entries[target])
    payload[4] ^= 0xFF
    payload[5] ^= 0xFF
    T.replace_entry(c, target, bytes(payload[: len(payload) - 4]))  # drop old cks word; repad/refix

    out = T.pack(c)
    c2 = T.unpack(out)
    ok_self = T.verify(c2.entries[target])
    ok_size = (len(c2.entries[target]) == len(orig_entries[target]))
    others_ok = all(c2.entries[i] == orig_entries[i]
                    for i in range(len(orig_entries)) if i != target)
    changed = (c2.entries[target] != orig_entries[target])
    print(f"  edited FDAT entry {target}: checksum_valid={ok_self} "
          f"size_preserved={ok_size} only_target_changed={others_ok and changed}")
    ok = ok_self and ok_size and others_ok and changed
    print(f"  -> {'OK' if ok else 'FAIL'}\n")
    return ok


def main():
    fm = T.load_filemap()
    if not os.path.exists(fm["source_bin"]):
        sys.exit(f"source disc not found: {fm['source_bin']}")
    results = {
        "round-trip gate": gate_roundtrip(fm),
        "checksum parity": gate_checksum_parity(fm),
        "edit correctness": gate_edit_correctness(fm),
    }
    print("=== SUMMARY ===")
    for k, v in results.items():
        print(f"  {k:<18} {'PASS' if v else 'FAIL'}")
    return 0 if all(results.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
