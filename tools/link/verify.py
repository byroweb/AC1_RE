"""verify.py UNIT=FILE ...: SHA-1 of each built code unit against overlays/CODE_UNITS.tsv.

UNIT is the CODE_UNITS.tsv unit name (SCUS_941.82 for the exe, fdatNNN for overlays).
Exits 1 if any unit differs, and prints the first differing offsets of each.
"""
import csv, hashlib, os, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def main():
    with open(os.path.join(ROOT, "overlays", "CODE_UNITS.tsv"), newline="") as fh:
        want = {r["unit"]: r["sha1_v1_0"] for r in csv.DictReader(fh, delimiter="\t")}
    bad = 0
    for arg in sys.argv[1:]:
        unit, path = arg.split("=", 1)
        data = open(path, "rb").read()
        got = hashlib.sha1(data).hexdigest()
        ok = got == want.get(unit)
        bad += not ok
        print("%-12s %s  %s  %s" % (unit, "OK  " if ok else "FAIL", got, os.path.relpath(path, ROOT)))
        if not ok:
            print("             want %s (%d bytes built)" % (want.get(unit, "?"), len(data)))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
