#!/usr/bin/env python3
"""p0_readiness.py - full-link readiness audit (docs/FULL_LINK_PLAN.md, phase P0).

    python tools/link/p0_readiness.py [-j N] [--out DIR] [--cache FILE]

Three checks, all read-only on the source tree:

  (a) every manifest row through the oracle with --json: MATCH without a masked
      word, and the effective address of every relocation;
  (b) each relocation target is sorted by where it lands, seen from the unit
      that references it: own rodata / text / data / bss, the exe, the host
      program overlay of a mission unit, the scratchpad / I/O, or fixed RAM
      outside every image;
  (c) function boundaries: the functions.tsv rows of each unit must tile its
      .text exactly; manifest sizes must agree with functions.tsv; every `jal`
      target and every data reference into a .text range must be a function
      start.

Writes <out>/data_refs.tsv (one row per distinct target address and class),
<out>/boundaries.tsv (gaps, overlaps, size disagreements, mid-function targets)
and <out>/summary.md.  The oracle results are cached by (row, source SHA-1) in
--cache so a rerun only recompiles changed rows.  Exit 0 when (a) and (c) are
clean.
"""
import argparse
import collections
import concurrent.futures as cf
import csv
import hashlib
import json
import os
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import drive  # noqa: E402
import units  # noqa: E402

MATCH_FN = os.path.join(REPO, "tools", "match", "match_fn.py")
CODE_UNITS = os.path.join(REPO, "overlays", "CODE_UNITS.tsv")
FUNCTIONS = os.path.join(REPO, "tools", "loop", "functions.tsv")

# exe .data / .bss split: __SN_ENTRY_POINT (0x80011E6C) zeroes 0x80039CB8..0x8004ADA0
EXE_BSS = (0x80039CB8, 0x8004ADA0)
# mission load address -> host program overlay (docs/MISSION_OVERLAY_CODE.md)
MISSION_HOST = {0x801C4B40: "ovl202", 0x801C8460: "ovl204", 0x801B80C8: "ovl203"}
# an overlay's .bss is taken to run from its image end up to the initial stack (PS-X EXE
# s_addr).  The mission load address is inside that range: 202/203/204 load missions at
# different addresses, and everything they reference above it moves by the same delta
# (P0 finding), so the mission buffer is itself a .bss array of the host build.
RAM_TOP = 0x801FFFF0
BSS_END = {}
SCRATCHPAD = (0x1F800000, 0x1F800400)
IO = (0x1F801000, 0x1F803000)


def h(x):
    return int(x, 16)


def load_units():
    """unit -> dict(vram, end, rodata, text, data) as [lo, hi) address pairs."""
    out = {}
    with open(CODE_UNITS, newline="") as fh:
        for r in csv.DictReader(fh, delimiter="\t"):
            if r["alias_of"]:
                continue
            name = "exe" if r["kind"] == "exe" else units.norm(r["unit"])
            vram, size = h(r["vram"]), h(r["size"])
            if name == "exe":                       # size counts the 0x800-byte header
                end = vram + size - 0x800
                data = (h(r["data_va"]), EXE_BSS[0])
                bss = EXE_BSS
            else:
                end = vram + size
                data = (h(r["data_va"]), end)
                bss = (end, BSS_END.get(name, RAM_TOP))
            out[name] = {"vram": vram, "end": end, "rodata": (vram, h(r["text_va"])),
                         "text": (h(r["text_va"]), h(r["text_end_va"])), "data": data, "bss": bss}
    return out


def unit_info(U, name):
    """U[name], adding units CODE_UNITS.tsv does not list (the fdat188 stub) from units.py."""
    if name not in U:
        ru = units.resolve(name)
        end = ru.base + os.path.getsize(ru.image_path)
        U[name] = {"vram": ru.base, "end": end, "rodata": (ru.base, ru.text_lo),
                   "text": (ru.text_lo, ru.text_hi), "data": (ru.text_hi, end), "bss": (end, RAM_TOP)}
    return U[name]


def load_functions():
    """unit -> sorted [(vaddr, size, is_psyq)]."""
    out = collections.defaultdict(list)
    with open(FUNCTIONS, newline="") as fh:
        for r in csv.DictReader(fh, delimiter="\t"):
            out[units.norm(r["unit"])].append((h(r["vaddr"]), int(r["size"]), r["is_psyq"] == "1"))
    for v in out.values():
        v.sort()
    return out


def inside(a, rng):
    return rng is not None and rng[0] <= a < rng[1]


def section_of(u, a):
    for sec in ("rodata", "text", "data", "bss"):
        if inside(a, u[sec]):
            return sec
    return None


# ---------------------------------------------------------------------------
# (a) oracle with --json
# ---------------------------------------------------------------------------
def oracle_argv(r, base):
    u = units.resolve(r["unit"] or "exe")
    extra = shlex.split(r["extra"]) if r["extra"] else []
    if "--base" in u.oracle_flags and "--base" in extra:
        i = extra.index("--base")
        del extra[i:i + 2]
    argv = [r["vaddr"], os.path.join(base, r["src"])] + list(u.oracle_flags)
    if r["size"]:
        argv += ["--expect-size", r["size"]]
    return argv + extra + ["-q", "--json", "-"]


def run_row(r, base):
    try:
        argv = oracle_argv(r, base)
    except (units.UnitError, ValueError) as e:
        return {"result": "ERROR", "error": str(e)}
    res = subprocess.run([sys.executable, MATCH_FN] + argv, cwd=REPO, capture_output=True,
                         text=True, errors="replace")
    out = res.stdout
    i, j = out.find("{"), out.rfind("}")
    if i < 0 or j < i:
        return {"result": "ERROR", "error": (out + res.stderr).strip()[-300:]}
    facts = json.loads(out[i:j + 1])
    return {k: facts.get(k) for k in ("result", "masked_words", "unresolved_symbols", "relocs",
                                       "bytes", "error")}


def cc1_sha1():
    """SHA-1 of the cc1 the oracle will use, so a rebuilt compiler invalidates the cache."""
    try:
        with open(os.path.join(REPO, "tools", "match", "vendor", "toolchain.json"), encoding="utf-8") as fh:
            path = json.load(fh)["cc1"]
        with open(path, "rb") as fh:
            return hashlib.sha1(fh.read()).hexdigest()
    except (OSError, KeyError, ValueError):
        return "unknown"


def row_key(r, base, cc1):
    p = os.path.join(base, r["src"])
    try:
        with open(p, "rb") as fh:
            sha = hashlib.sha1(fh.read()).hexdigest()
    except OSError:
        sha = "missing"
    return "\t".join((r["vaddr"], r["unit"], r["src"], r["size"], r["extra"], sha, cc1))


def run_oracle(rows, base, jobs, cache_path):
    cache = {}
    if cache_path and os.path.isfile(cache_path):
        with open(cache_path, encoding="utf-8") as fh:
            cache = json.load(fh)
    cc1 = cc1_sha1()
    keys = [row_key(r, base, cc1) for r in rows]
    # an ERROR (e.g. a cc1 crash) is retried on every run instead of being trusted from the cache
    todo = [i for i, k in enumerate(keys) if cache.get(k, {}).get("result") in (None, "ERROR")]
    print("oracle: %d row(s), %d cached, %d to run" % (len(rows), len(rows) - len(todo), len(todo)),
          file=sys.stderr)
    with cf.ThreadPoolExecutor(max_workers=max(1, jobs)) as ex:
        for n, (i, facts) in enumerate(zip(todo, ex.map(lambda i: run_row(rows[i], base), todo)), 1):
            cache[keys[i]] = facts
            if n % 500 == 0:
                print("  %d/%d" % (n, len(todo)), file=sys.stderr)
    if cache_path:
        os.makedirs(os.path.dirname(os.path.abspath(cache_path)), exist_ok=True)
        live = set(keys)
        with open(cache_path, "w", encoding="utf-8") as fh:
            json.dump({k: v for k, v in cache.items() if k in live}, fh)
    return [cache[k] for k in keys]


# ---------------------------------------------------------------------------
# (b) classify targets
# ---------------------------------------------------------------------------
def classify(unit, a, U):
    """-> (class, owner unit or '')."""
    own = unit_info(U, unit)
    sec = section_of(own, a)
    if sec:
        return "own-" + sec, unit
    if unit != "exe":
        sec = section_of(U["exe"], a)
        if sec:
            return "exe-" + sec, "exe"
    if unit.startswith("fdat"):
        host = MISSION_HOST.get(own["vram"])
        if host:
            sec = section_of(U[host], a)
            if sec:
                return "host-" + sec, host
    if unit == "exe":                            # the boot loop calls into the front end it loads
        sec = section_of(U["ovl201"], a)
        if sec:
            return "overlay-" + sec, "ovl201"
    if inside(a, SCRATCHPAD):
        return "scratchpad", ""
    if inside(a, IO):
        return "io", ""
    return "fixed-ram", ""


# ---------------------------------------------------------------------------
# (c) boundaries
# ---------------------------------------------------------------------------
def tiling(U, F):
    issues = []
    for unit, fns in sorted(F.items()):
        if unit not in U:
            continue
        lo, hi = U[unit]["text"]
        try:
            ru = units.resolve(unit)
            with open(ru.image_path, "rb") as fh:
                img = fh.read()
            ibase = ru.base
        except Exception:                         # noqa: BLE001 - the image is optional here
            img, ibase = None, None

        def zeros(a, b):
            if img is None:
                return None
            return not any(img[a - ibase:b - ibase])

        cur = lo
        for va, size, psyq in fns:
            if va > cur:
                z = zeros(cur, va)
                issues.append((unit, "gap", "0x%08X" % cur, va - cur,
                               "zero padding" if z else ("non-zero bytes" if z is False else "?"),
                               "psyq" if psyq else ""))
            elif va < cur:
                issues.append((unit, "overlap", "0x%08X" % va, cur - va, "", "psyq" if psyq else ""))
            cur = max(cur, va + size)
        if cur != hi:
            issues.append((unit, "tail", "0x%08X" % cur, hi - cur,
                           "past .text end" if cur > hi else
                           ("zero padding" if zeros(cur, hi) else "non-zero bytes"), ""))
    return issues


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--out", default=os.path.join(REPO, "build", "link", "p0"))
    ap.add_argument("--cache", default=os.path.join(REPO, "build", "link", "p0", "oracle_cache.json"))
    ap.add_argument("--manifest", default=drive.MANIFEST)
    args = ap.parse_args(argv)
    os.makedirs(args.out, exist_ok=True)

    U, F = load_units(), load_functions()
    starts = {u: {va for va, _, _ in fns} for u, fns in F.items()}
    sizes = {(u, va): s for u, fns in F.items() for va, s, _ in fns}
    _hdr, rows, _crlf = drive.read_manifest(args.manifest)
    base = os.path.dirname(os.path.abspath(args.manifest))
    facts = run_oracle(rows, base, args.jobs, args.cache)

    # (a)
    bad, masked = [], []
    for r, f in zip(rows, facts):
        if f.get("result") != "MATCH":
            bad.append((r, f))
        elif f.get("masked_words"):
            masked.append((r, f))

    # (b) + (c) targets
    refs = collections.defaultdict(lambda: {"rows": set(), "syms": set(), "kinds": set()})
    mids = []
    for r, f in zip(rows, facts):
        unit = units.norm(r["unit"] or "exe")
        va = h(r["vaddr"])
        size = int(r["size"]) if r["size"] else (f.get("bytes") or 0)
        for rel in f.get("relocs") or []:
            if rel["addr"] is None:
                continue
            a = h(rel["addr"])
            if va <= a < va + size:              # branch to a local label / own jump table entry
                continue
            cls, owner = classify(unit, a, U)
            kind = "call" if rel["type"] == "26" else "data"
            # the owning unit; fixed RAM / I/O have no owner, so they are listed under the referrer
            key = (owner or unit, cls, a)
            d = refs[key]
            d["rows"].add((unit, r["vaddr"]))
            d["syms"].add(rel["sym"])
            d["kinds"].add(kind)
            if cls.endswith("-text") and a not in starts.get(owner, ()):
                mids.append((unit, r["vaddr"], kind, rel["sym"], "0x%08X" % a, owner))

    size_bad = []
    for r in rows:
        unit = units.norm(r["unit"] or "exe")
        s = sizes.get((unit, h(r["vaddr"])))
        if s is None:
            size_bad.append((unit, r["vaddr"], r["size"], "not in functions.tsv"))
        elif r["size"] and int(r["size"]) != s:
            size_bad.append((unit, r["vaddr"], r["size"], "functions.tsv says %d" % s))
    tiles = tiling(U, F)

    with open(os.path.join(args.out, "data_refs.tsv"), "w", newline="", encoding="utf-8") as fh:
        w = csv.writer(fh, delimiter="\t", lineterminator="\n")
        w.writerow(["unit", "class", "addr", "kinds", "n_functions", "symbols"])
        for (u, cls, a), d in sorted(refs.items(), key=lambda kv: (kv[0][0], kv[0][2], kv[0][1])):
            w.writerow([u, cls, "0x%08X" % a, ",".join(sorted(d["kinds"])), len(d["rows"]),
                        " ".join(sorted(d["syms"]))[:200]])
    with open(os.path.join(args.out, "boundaries.tsv"), "w", newline="", encoding="utf-8") as fh:
        w = csv.writer(fh, delimiter="\t", lineterminator="\n")
        w.writerow(["unit", "issue", "addr", "bytes_or_ref", "detail", "note"])
        for t in tiles:
            w.writerow(t)
        for unit, vaddr, msize, why in size_bad:
            w.writerow([unit, "size", vaddr, msize, why, ""])
        for unit, vaddr, kind, sym, a, owner in mids:
            w.writerow([unit, "mid-function-" + kind, a, "from " + vaddr, sym, owner])

    # summary
    by_class = collections.Counter()
    by_class_refs = collections.Counter()
    for (u, cls, a), d in refs.items():
        by_class[cls] += 1
        by_class_refs[cls] += len(d["rows"])
    tile_kinds = collections.Counter((t[1], t[4]) for t in tiles)
    lines = ["# P0 readiness — generated by tools/link/p0_readiness.py", "",
             "## (a) oracle", "",
             "- manifest rows: %d" % len(rows),
             "- clean MATCH: %d" % (len(rows) - len(bad) - len(masked)),
             "- MATCH with masked words: %d" % len(masked),
             "- not MATCH (MISMATCH / ERROR): %d" % len(bad), ""]
    for r, f in (bad + masked)[:40]:
        lines.append("  - %s %s %s: %s %s" % (r["unit"], r["vaddr"], r["src"], f.get("result"),
                                             f.get("error") or ", ".join(f.get("unresolved_symbols") or [])))
    lines += ["", "## (b) relocation targets by class", "",
              "| class | distinct addresses | (function, address) references |", "|---|---:|---:|"]
    for cls, n in sorted(by_class.items(), key=lambda kv: -kv[1]):
        lines.append("| %s | %d | %d |" % (cls, n, by_class_refs[cls]))
    lines += ["", "## (c) boundaries", "",
              "- tiling issues: " + ", ".join("%s/%s %d" % (k[0], k[1], n) for k, n in sorted(tile_kinds.items())),
              "- manifest size disagreements: %d" % len(size_bad),
              "- mid-function targets (calls/data into .text but not a function start): %d" % len(mids), ""]
    with open(os.path.join(args.out, "summary.md"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n")
    print("\n".join(lines))
    return 0 if not bad and not masked and not size_bad and not mids else 1


if __name__ == "__main__":
    sys.exit(main())
