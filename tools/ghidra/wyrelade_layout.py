#!/usr/bin/env python3
"""wyrelade_layout.py — layout + hash of every AC1 code unit, from the Wyrelade decomp.

SLUS-01323 (v1.1) and SCUS-94182 (v1.0) share byte-identical code, so the splat
configs of the CC0 Wyrelade/Armored-Core-Decomp repo describe our executable and
all 62 FDAT.T code overlays exactly. This script fetches those configs AT RUN TIME
(nothing vendored) and distils the facts we are allowed to borrow:

  unit, FDAT entry, load address, size, .rodata / .text / .data boundaries,
  SHA-1 of the v1.0 unit, and which mission overlay is a byte-identical alias.

No C, headers or build scripts are read. Only configs/USA/*.yaml and
configs/USA/mission_overlays.txt.

Usage (repo root):
  python tools/ghidra/wyrelade_layout.py                   # TSV to stdout
  python tools/ghidra/wyrelade_layout.py --out overlays/CODE_UNITS.tsv
  python tools/ghidra/wyrelade_layout.py --md              # Markdown table instead
  python tools/ghidra/wyrelade_layout.py --check           # carve every unit from $AC1_BIN
                                                           # and compare SHA-1 with v1.0
  python tools/ghidra/wyrelade_layout.py --cache DIR       # keep the downloaded configs

--check reads GG/COM/FDAT.T and the boot exe straight from your own disc image
(tools/extract/extract_overlay.py machinery, $AC1_BIN via disc_map/disc_files.json)
and reports identical / different per unit. That is the reproducible form of the
"v1.1 code == v1.0 code" claim in docs/REFERENCE.md.
"""
import argparse
import hashlib
import os
import re
import sys
import urllib.request

RAW = "https://raw.githubusercontent.com/Wyrelade/Armored-Core-Decomp/main/configs/USA/"
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
PROGRAM_UNITS = ["fdat201", "fdat202", "fdat203", "fdat204"]
EXE_UNIT = "SCUS_941.82"
EXE_FILE_V11 = "SLUS_013.23"


def fetch(name, cache=None):
    if cache:
        p = os.path.join(cache, name)
        if os.path.exists(p):
            with open(p, encoding="utf-8") as f:
                return f.read()
    req = urllib.request.Request(RAW + name, headers={"User-Agent": "AC1_RE wyrelade_layout.py"})
    with urllib.request.urlopen(req, timeout=30) as r:
        text = r.read().decode("utf-8")
    if cache:
        os.makedirs(cache, exist_ok=True)
        with open(os.path.join(cache, name), "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
    return text


def parse_yaml(text):
    """Minimal reader for the splat configs: sha1, vram, section offsets, size."""
    u = {"sha1": None, "vram": None, "rodata": None, "text": None, "data": None,
         "size": None, "file_start": 0}
    m = re.search(r"^sha1:\s*([0-9a-f]{40})", text, re.M)
    if m:
        u["sha1"] = m.group(1)
    m = re.search(r"^\s*vram:\s*(0x[0-9A-Fa-f]+)", text, re.M)
    if m:
        u["vram"] = int(m.group(1), 16)
    m = re.search(r"^\s*type:\s*code\s*\n\s*start:\s*(0x[0-9A-Fa-f]+)", text, re.M)
    if m:
        u["file_start"] = int(m.group(1), 16)
    for off, kind in re.findall(r"^\s*-\s*\[(0x[0-9A-Fa-f]+),\s*([.\w]+)", text, re.M):
        o = int(off, 16)
        if kind == ".rodata" and u["rodata"] is None:
            u["rodata"] = o
        elif kind in ("c", "asm", ".text") and u["text"] is None:
            u["text"] = o
        elif kind in ("data", ".data") and u["data"] is None:
            u["data"] = o
    # the trailing "- [0xSIZE]" line is the unit's end
    ends = re.findall(r"^\s*-\s*\[(0x[0-9A-Fa-f]+)\]\s*$", text, re.M)
    if ends:
        u["size"] = int(ends[-1], 16)
    return u


def parse_missions(text):
    """mission_overlays.txt: unit vram sha1 [alias-of]."""
    rows = []
    for ln in text.splitlines():
        ln = ln.strip()
        if not ln or ln.startswith("#"):
            continue
        p = ln.split()
        rows.append({"unit": p[0], "vram": int(p[1], 16), "sha1": p[2],
                     "alias_of": p[3] if len(p) > 3 else ""})
    return rows


def entry_of(unit):
    m = re.fullmatch(r"fdat(\d+)", unit)
    return int(m.group(1)) if m else None


def collect(cache):
    units = []
    exe = parse_yaml(fetch(EXE_UNIT + ".yaml", cache))
    exe.update(unit=EXE_UNIT, entry=None, alias_of="", kind="exe")
    units.append(exe)
    for name in PROGRAM_UNITS:
        u = parse_yaml(fetch(name + ".yaml", cache))
        u.update(unit=name, entry=entry_of(name), alias_of="", kind="program")
        units.append(u)
    missions = parse_missions(fetch("mission_overlays.txt", cache))
    for mrow in missions:
        name = mrow["unit"]
        if mrow["alias_of"]:
            # byte-identical twin: same layout as the unit it aliases (no own yaml)
            src = next(x for x in units if x["unit"] == mrow["alias_of"])
            u = dict(src)
        else:
            u = parse_yaml(fetch(name + ".yaml", cache))
        u.update(unit=name, entry=entry_of(name), kind="mission",
                 alias_of=mrow["alias_of"], sha1=mrow["sha1"], vram=mrow["vram"])
        units.append(u)
    return units


def va(u, off):
    """file offset -> virtual address (exe has a 0x800 header before vram)."""
    if off is None or u["vram"] is None:
        return None
    return u["vram"] + (off - u["file_start"])


COLS = ["unit", "entry", "kind", "vram", "size", "rodata_off", "text_off", "data_off",
        "text_va", "text_end_va", "data_va", "sha1_v1_0", "alias_of"]


def row_values(u):
    def hx(v, w=0):
        return "" if v is None else (f"0x{v:0{w}X}")
    return [u["unit"], "" if u["entry"] is None else str(u["entry"]), u["kind"],
            hx(u["vram"], 8), hx(u["size"]), hx(u["rodata"]), hx(u["text"]), hx(u["data"]),
            hx(va(u, u["text"]), 8), hx(va(u, u["data"]), 8), hx(va(u, u["data"]), 8),
            u["sha1"] or "", u["alias_of"]]


def emit(units, md):
    if md:
        yield "| " + " | ".join(COLS) + " |"
        yield "|" + "---|" * len(COLS)
        for u in units:
            yield "| " + " | ".join(f"`{v}`" if v else "" for v in row_values(u)) + " |"
    else:
        yield "\t".join(COLS)
        for u in units:
            yield "\t".join(row_values(u))


def check(units):
    """Carve each unit from the user's own disc and compare SHA-1 with v1.0."""
    sys.path.insert(0, os.path.join(REPO, "tools", "extract"))
    import extract_overlay                 # noqa: E402  (repo tool)
    import extract_t                       # noqa: E402
    import json
    container = extract_overlay.load_container(disc=True)
    count, offsets = extract_overlay.read_toc(container)
    fm = json.load(open(os.path.join(REPO, "disc_map", "disc_files.json")))
    disc = extract_t.disc_bin(fm)

    def exe_bytes():
        rec = next((r for r in fm["files"] if r.get("path", "").endswith(EXE_FILE_V11)
                    or r.get("path", "").endswith(EXE_UNIT)), None)
        if rec is None:
            return None
        raw = extract_t.read_sectors(disc, rec["sector_first"],
                                     rec["sector_last"] - rec["sector_first"] + 1)
        size = rec.get("size") or rec.get("bytes")
        if not size:
            # PS-X EXE: 0x800 header + t_size at +0x1C
            size = 0x800 + int.from_bytes(raw[0x1C:0x20], "little")
        return raw[:size]

    same = diff = missing = 0
    for u in units:
        if u["kind"] == "exe":
            data = exe_bytes()
        else:
            e = u["entry"]
            if e is None or e + 1 >= len(offsets):
                data = None
            else:
                s0, s1 = offsets[e], offsets[e + 1]
                data = container[s0 * 2048:s1 * 2048]
        if data is None:
            print(f"  {u['unit']:<12} ?        (not found on disc)")
            missing += 1
            continue
        h = hashlib.sha1(data).hexdigest()
        ok = (h == u["sha1"])
        same += ok
        diff += (not ok)
        note = "" if ok else f"  v1.1={h} v1.0={u['sha1']}"
        size_note = "" if u["size"] in (None, len(data)) else f"  size {len(data):#x} != yaml {u['size']:#x}"
        print(f"  {u['unit']:<12} {'IDENTICAL' if ok else 'DIFFERENT'}  {len(data):>7,} B{note}{size_note}")
    print(f"\n{same} identical, {diff} different, {missing} not found "
          f"(of {len(units)} units: exe + {len(units) - 1} FDAT.T code entries)")
    return diff == 0 and missing == 0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", help="write the table here instead of stdout")
    ap.add_argument("--md", action="store_true", help="Markdown table instead of TSV")
    ap.add_argument("--cache", help="directory to keep/reuse the downloaded configs")
    ap.add_argument("--check", action="store_true",
                    help="carve every unit from $AC1_BIN and compare SHA-1 with v1.0")
    a = ap.parse_args()

    units = collect(a.cache)
    text = "\n".join(emit(units, a.md)) + "\n"
    if a.out:
        with open(a.out, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        print(f"{len(units)} units -> {a.out}", file=sys.stderr)
    else:
        sys.stdout.write(text)
    if a.check:
        ok = check(units)
        sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
