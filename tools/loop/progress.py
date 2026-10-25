#!/usr/bin/env python3
"""progress.py - matched functions per component, as a markdown table.

    progress.py [--manifest re/match/manifest.tsv] [--functions F] [--compare] [--stubs-matched]

Prints the table in the shape of the progress table in the README of
Wyrelade/Armored-Core-Decomp (components: the exe's game code, the program
overlays with FDAT_201..204 under them, the mission overlays, the total), so
the two projects can be read side by side.  Only the layout is shared; the
numbers are ours:

  Functions  rows of tools/loop/functions.tsv with is_psyq 0 (inventory.py
             --missions; fdat168 is an alias of fdat166 and is not counted twice)
  Matched    distinct (unit, vaddr) rows of re/match/manifest.tsv that are a
             function in functions.tsv - originals and replayed twins alike,
             every one re-verified by check_all.sh / drive.py verify.  Manifest
             rows that are not (or no longer) a functions.tsv start are counted
             on a separate line, not in the table.
  Provisional  matched rows listed in re/match/provisional.tsv (C that steers the
             compiler with asm pins; provisional.py) are counted as matched and
             reported on their own line with the clean total.

--stubs-matched also counts the open 8-byte `jr ra; nop` functions as matched
(his README says the empty stubs splat wrote as C count as matched there).

--compare fetches his README through `gh api` (repos/Wyrelade/Armored-Core-Decomp)
and prints his badge and per-component counts next to ours with the deltas.
Only numbers are read from it.  Function boundaries differ between the two
splits (his exe game code is 147 functions, ours 131), so compare percentages
as well as counts.
"""
import argparse
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import merge_manifest  # noqa: E402
import provisional  # noqa: E402
import units  # noqa: E402

FUNCTIONS = os.path.join(HERE, "functions.tsv")
MANIFEST = os.path.join(REPO, "re", "match", "manifest.tsv")
UPSTREAM = "repos/Wyrelade/Armored-Core-Decomp/readme"
PROGRAMS = ("ovl201", "ovl202", "ovl203", "ovl204")
BAR = 20


def bar(done, total):
    pct = 100.0 * done / total if total else 0.0
    k = int(round(BAR * done / total)) if total else 0
    return "`%s%s` %.2f%%" % ("▰" * k, "▱" * (BAR - k), pct)


def load_functions(path):
    """{unit: {vaddr: size}} for every non-PsyQ function."""
    out = {}
    with open(path, encoding="utf-8") as fh:
        header = fh.readline().rstrip("\r\n").split("\t")
        col = {h: i for i, h in enumerate(header)}
        for line in fh:
            p = line.rstrip("\r\n").split("\t")
            if len(p) < len(header) or p[col["is_psyq"]] == "1":
                continue
            out.setdefault(p[col["unit"]], {})[int(p[col["vaddr"]], 16)] = int(p[col["size"]])
    return out


def load_matched(path):
    """{(unit, vaddr)} from the manifest (normalized unit names)."""
    _h, rows = merge_manifest.parse(open(path, encoding="utf-8").read())
    out = set()
    for r in rows:
        try:
            out.add((units.norm(r[1] or "exe"), int(r[0], 16)))
        except (ValueError, units.UnitError):
            continue
    return out


def count(funcs, matched, stubs):
    """{unit: (functions, matched)} and the manifest keys outside functions.tsv."""
    per = {}
    for unit, fs in funcs.items():
        m = sum(1 for va, sz in fs.items() if (unit, va) in matched or (stubs and sz == 8))
        per[unit] = (len(fs), m)
    stray = sorted(k for k in matched if k[1] not in funcs.get(k[0], {}))
    return per, stray


def components(per):
    """[(key, label, functions, matched)] in table order."""
    def tot(keys):
        f = sum(per.get(k, (0, 0))[0] for k in keys)
        return f, sum(per.get(k, (0, 0))[1] for k in keys)
    missions = sorted(k for k in per if k.startswith("fdat"))
    rows = [("exe", "**Main executable** (`SLUS_013.23`, game code)") + tot(["exe"]),
            ("programs", "**Program overlays** (`FDAT.T`)") + tot(PROGRAMS)]
    for u in PROGRAMS:
        rows.append((u, "&nbsp;&nbsp;└ `FDAT_%s`" % u[3:]) + tot([u]))
    rows.append(("missions", "**Mission overlays** (`FDAT.T`, %d entries)" % len(missions)) + tot(missions))
    rows.append(("total", "**Total**") + tot(["exe"] + list(PROGRAMS) + missions))
    return rows


def render(rows):
    out = ["| Component | Functions | Matched | Progress |", "|---|---:|---:|---|"]
    for _k, label, f, m in rows:
        out.append("| %s | %d | %d | %s |" % (label, f, m, bar(m, f)))
    return "\n".join(out)


def fetch_upstream():
    """(badge text or None, {key: (functions, matched)}) from his README, or raises."""
    res = subprocess.run(["gh", "api", UPSTREAM, "-H", "Accept: application/vnd.github.raw"],
                         capture_output=True, text=True, encoding="utf-8", errors="replace")
    if res.returncode != 0:
        raise RuntimeError(res.stderr.strip() or "gh api failed")
    text = res.stdout
    badge = None
    m = re.search(r"badge/matched-([0-9]+)%2F([0-9]+)", text)
    if m:
        badge = "%s/%s" % (m.group(1), m.group(2))
    table = {}
    block = re.search(r"<!-- PROGRESS:TABLE -->(.*?)<!-- /PROGRESS:TABLE -->", text, re.S)
    for line in (block.group(1) if block else "").splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) < 3 or not cells[1].isdigit():
            continue
        label, f = cells[0], int(cells[1])
        mt = int(cells[2]) if cells[2].isdigit() else None
        if mt is None:
            continue                                   # the PsyQ row: not counted
        lk = label.lower()
        fd = re.search(r"fdat_(20[1-4])", lk)
        key = ("ovl" + fd.group(1) if fd else "exe" if "main executable" in lk else
               "programs" if "program overlays" in lk else "missions" if "mission overlays" in lk else
               "total" if "total" in lk else None)
        if key:
            table[key] = (f, mt)
    return badge, table


def render_compare(rows, theirs):
    out = ["| Component | ours | Wyrelade | Δ matched | ours % | Wyrelade % |",
           "|---|---:|---:|---:|---:|---:|"]
    for k, label, f, m in rows:
        t = theirs.get(k)
        if not t:
            out.append("| %s | %d/%d | - | - | %.2f%% | - |" % (label, m, f, 100.0 * m / f if f else 0))
            continue
        out.append("| %s | %d/%d | %d/%d | %+d | %.2f%% | %.2f%% |"
                   % (label, m, f, t[1], t[0], m - t[1], 100.0 * m / f if f else 0,
                      100.0 * t[1] / t[0] if t[0] else 0))
    return "\n".join(out)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--manifest", default=MANIFEST)
    ap.add_argument("--functions", default=FUNCTIONS)
    ap.add_argument("--stubs-matched", action="store_true",
                    help="count open 8-byte `jr ra; nop` functions as matched (his convention)")
    ap.add_argument("--compare", action="store_true", help="fetch Wyrelade's README table (gh api) and print the delta")
    args = ap.parse_args(argv)

    funcs, matched = load_functions(args.functions), load_matched(args.manifest)
    per, stray = count(funcs, matched, args.stubs_matched)
    rows = components(per)
    print(render(rows))
    prov = {k for k in provisional.load() if k in matched and k[1] in funcs.get(k[0], {})}
    if prov:
        total, done = rows[-1][2], rows[-1][3]
        print("\n%d of the matched are provisional (C steered with asm pins, re/match/provisional.tsv);"
              " clean: %d / %d (%.2f%%)" % (len(prov), done - len(prov), total, 100.0 * (done - len(prov)) / total))
    if stray:
        print("\n%d manifest row(s) are not a functions.tsv start (not counted): %s"
              % (len(stray), ", ".join("%s 0x%08X" % k for k in stray[:8]) + (" ..." if len(stray) > 8 else "")))
    if args.stubs_matched:
        print("\n(open 8-byte `jr ra; nop` stubs counted as matched)")
    if args.compare:
        try:
            badge, theirs = fetch_upstream()
        except (RuntimeError, OSError) as e:
            print("\n--compare: could not fetch %s: %s" % (UPSTREAM, e), file=sys.stderr)
            return 1
        print("\nWyrelade/Armored-Core-Decomp README: badge %s" % (badge or "?"))
        print(render_compare(rows, theirs))
    return 0


if __name__ == "__main__":
    sys.exit(main())
