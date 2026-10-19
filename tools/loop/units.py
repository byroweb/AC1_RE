#!/usr/bin/env python3
"""units.py - one place that knows how a unit name maps onto an image and oracle flags.

A unit is `exe`, `ovl201`..`ovl204` or `fdatNNN` (mission overlay, FDAT_T
entry NNN).  Everything here is derived from inventory.py (which the mission
session owns; only load_unit / find_in_disc_map / find_objdump /
MISSION_ENTRIES / load_code_units are imported, by name) plus
overlays/CODE_UNITS.tsv when that file exists.

    units.py flags <unit>     the match_fn.py flags that select the right binary
    units.py image <unit>     absolute path of the unit's image file
    units.py base  <unit>     load address of byte 0 of that image, 0x%08X

    exe      -> no flags (exe addresses are unambiguous to the oracle)
    ovl20N   -> --ovl 20N
    fdatNNN  -> --bin <abs path> --base 0x801C4B40 (etc.)

Alias units (fdat168 is byte-identical to fdat166) are refused: match them
under their canonical name.  The exe image is the PS-X EXE carved from $AC1_BIN,
cached in tools/loop/cache/ (gitignored via *.bin).
"""
import json
import os
import re
import sys
from collections import namedtuple

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
from inventory import (load_unit, find_in_disc_map, find_objdump,  # noqa: E402,F401
                       MISSION_ENTRIES, load_code_units)

CODE_UNITS = os.path.join(REPO, "overlays", "CODE_UNITS.tsv")
EXE_CACHE = os.path.join(HERE, "cache", "ac1_exe_carved.bin")
EXE_FILE_BASE = 0x80010800          # file offset 0 of the carved exe (header at -0x800)
EXE_TEXT = (0x80011E64, 0x800315B4)
PROGRAM_OVERLAYS = (201, 202, 203, 204)
# known aliases when CODE_UNITS.tsv is not checked out
ALIASES = {"fdat168": "fdat166"}

Unit = namedtuple("Unit", "name image_path base text_lo text_hi oracle_flags manifest_unit")


class UnitError(Exception):
    pass


def norm(unit):
    """exe | ovl201 | fdat060 from the spellings people type (201, fdat60, ...)."""
    u = str(unit).strip().lower()
    if u in ("exe", "main", "slus"):
        return "exe"
    m = re.fullmatch(r"(?:ovl|fdat)?0*(\d+)", u)
    if not m:
        raise UnitError("bad unit %r (exe, ovl201..ovl204, fdatNNN)" % unit)
    n = int(m.group(1))
    return "ovl%d" % n if n in PROGRAM_OVERLAYS else "fdat%03d" % n


def _code_units_table():
    """{unit: {vram, alias_of}} from overlays/CODE_UNITS.tsv (empty if absent)."""
    out = {}
    if not os.path.isfile(CODE_UNITS):
        return out
    with open(CODE_UNITS, encoding="utf-8") as fh:
        header = fh.readline().rstrip("\r\n").split("\t")
        col = {h: i for i, h in enumerate(header)}
        for line in fh:
            parts = line.rstrip("\r\n").split("\t")
            if len(parts) < len(header):
                parts += [""] * (len(header) - len(parts))
            kind = parts[col.get("kind", 2)]
            entry = parts[col.get("entry", 1)]
            if kind == "exe":
                name = "exe"
            elif entry.isdigit():
                n = int(entry)
                name = "ovl%d" % n if n in PROGRAM_OVERLAYS else "fdat%03d" % n
            else:
                continue
            row = {"vram": None, "alias_of": ""}
            try:
                row["vram"] = int(parts[col["vram"]], 16)
            except (KeyError, ValueError):
                pass
            if "alias_of" in col:
                row["alias_of"] = parts[col["alias_of"]].strip()
            out[name] = row
    return out


def _abs(p):
    return os.path.abspath(p).replace("\\", "/")


def _ovl_image(n):
    j = find_in_disc_map(os.path.join("overlays", "overlays.json"))
    if not j:
        raise UnitError("disc_map/overlays/overlays.json not found (set AC1_DISC_MAP)")
    with open(j) as fh:
        meta = json.load(fh)
    for o in meta.get("overlays", []):
        if o.get("entry") == n:
            p = os.path.join(os.path.dirname(j), os.path.basename(o["bin"].replace("\\", "/")))
            if not os.path.isfile(p):
                raise UnitError("overlay image %s missing" % p)
            return _abs(p), int(o.get("load_base", 0x8004ADA0))
    raise UnitError("overlay %d not in overlays.json" % n)


def _exe_image():
    if not os.path.isfile(EXE_CACHE):
        img, base, _label = load_unit("exe")
        os.makedirs(os.path.dirname(EXE_CACHE), exist_ok=True)
        with open(EXE_CACHE, "wb") as fh:
            fh.write(img)
    return _abs(EXE_CACHE)


def resolve(unit, need_image=True):
    """Unit(...) for a unit name.  need_image=False skips carving/looking up the file."""
    name = norm(unit)
    table = _code_units_table()
    text = load_code_units(CODE_UNITS)
    if name == "exe":
        lo, hi = text.get("exe", EXE_TEXT)
        return Unit("exe", _exe_image() if need_image else None, EXE_FILE_BASE, lo, hi, [], "exe")
    m = re.fullmatch(r"ovl(\d+)", name)
    if m:
        n = int(m.group(1))
        path, base = _ovl_image(n) if need_image else (None, 0x8004ADA0)
        lo, hi = text.get(name, (None, None))
        return Unit(name, path, base, lo, hi, ["--ovl", str(n)], str(n))
    m = re.fullmatch(r"fdat(\d+)", name)
    n = int(m.group(1))
    alias = table.get(name, {}).get("alias_of") or ALIASES.get(name)
    if alias:
        raise UnitError("%s is a byte-identical alias of %s; use %s" % (name, alias, alias))
    base = table.get(name, {}).get("vram")
    if base is None:
        if n not in MISSION_ENTRIES:
            raise UnitError("no known load address for FDAT entry %d" % n)
        base = MISSION_ENTRIES[n]
    path = None
    if need_image:
        path = find_in_disc_map(os.path.join("extracted", "FDAT_T", "entries", "%03d.bin" % n))
        if not path:
            raise UnitError("disc_map/extracted/FDAT_T/entries/%03d.bin not found (set AC1_DISC_MAP)" % n)
        path = _abs(path)
    lo, hi = text.get(name, (None, None))
    flags = ["--bin", path if path else "<%s image>" % name, "--base", "0x%08X" % base]
    return Unit(name, path, base, lo, hi, flags, name)


def main(argv):
    if len(argv) != 2 or argv[0] not in ("flags", "image", "base"):
        print(__doc__)
        return 2
    cmd, unit = argv
    try:
        u = resolve(unit, need_image=(cmd != "base"))
    except (UnitError, SystemExit) as e:
        print("units.py: %s" % e, file=sys.stderr)
        return 1
    if cmd == "flags":
        print(" ".join(u.oracle_flags))
    elif cmd == "image":
        print(u.image_path)
    else:
        print("0x%08X" % u.base)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
