#!/usr/bin/env python3
"""psyq_labels.py — PsyQ library names for the AC1 executable, from the Wyrelade decomp.

SLUS-01323 (v1.1) and SCUS-94182 (v1.0) share byte-identical code (docs/REFERENCE.md
section 1, Revision note), so the signature-identified PsyQ library list that the CC0
Wyrelade/Armored-Core-Decomp repo keeps in configs/USA/psyq_funcs.txt applies to our
exe address for address. This script fetches that file AT RUN TIME (it is not
vendored), writes a Ghidra-importable label list, and reports what it would add to
or correct in our ac1_markup.tsv. It never touches ac1_markup_pristine.tsv.

Usage (from the repo root):
  python tools/ghidra/psyq_labels.py                       # labels -> stdout, report -> stderr
  python tools/ghidra/psyq_labels.py --out psyq_labels.txt # Ghidra: Script Manager -> ImportSymbolsScript.py
  python tools/ghidra/psyq_labels.py --format tsv          # ac1_markup-style F rows instead
  python tools/ghidra/psyq_labels.py --source path/psyq_funcs.txt   # offline copy
  python tools/ghidra/psyq_labels.py --merged ac1_markup_psyq.tsv   # markup copy with names applied

Output formats
  ghidra (default): "<name> 0x<addr> f" per line, the format Ghidra's bundled
                    ImportSymbolsScript.py reads (f = create a function + label).
  tsv:              "F<tab><addr><tab><name><tab>unknown<tab>undefined <name>(void)"
                    like ac1_markup.tsv.

Static / unnamed library functions ('-' in the psyq-name column) are emitted under
their func_XXXXXXXX symbol so the 528 count is complete; --named-only drops them.
"""
import argparse
import os
import re
import sys
import tempfile
import urllib.request

RAW_URL = ("https://raw.githubusercontent.com/Wyrelade/Armored-Core-Decomp/"
           "main/configs/USA/psyq_funcs.txt")
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
MARKUP = os.path.join(REPO, "ac1_markup.tsv")
PRISTINE = os.path.join(REPO, "ac1_markup_pristine.tsv")

# Ghidra / our own placeholder names that a real PsyQ name should replace.
GENERIC = re.compile(r"^(FUN_|LAB_|SUB_|thunk_|func_|[A-Z0-9]+_OBJ_[0-9A-Fa-f]+$)")
EXE_LO, EXE_HI = 0x80011000, 0x8003A000           # SLUS code range (REFERENCE section 1)
LIB_LO, LIB_HI = 0x80016EA4, 0x80030368           # contiguous PsyQ library block (psyq_funcs.txt header)


def fetch(source, cache):
    """Return the text of psyq_funcs.txt from a local path or (downloaded) URL."""
    if source and not re.match(r"^https?://", source):
        with open(source, encoding="utf-8") as f:
            return f.read()
    url = source or RAW_URL
    if cache and os.path.exists(cache):
        with open(cache, encoding="utf-8") as f:
            return f.read()
    req = urllib.request.Request(url, headers={"User-Agent": "AC1_RE psyq_labels.py"})
    with urllib.request.urlopen(req, timeout=30) as r:
        text = r.read().decode("utf-8")
    if cache:
        os.makedirs(os.path.dirname(cache) or ".", exist_ok=True)
        with open(cache, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
    return text


def parse_psyq(text):
    """-> list of dicts {symbol, addr, lib, obj, name} (name = psyq name or None)."""
    out = []
    for ln in text.splitlines():
        ln = ln.strip()
        if not ln or ln.startswith("#"):
            continue
        parts = ln.split()
        if len(parts) < 5:
            continue
        sym, addr, lib, obj, pname = parts[:5]
        out.append({"symbol": sym, "addr": int(addr, 16), "lib": lib, "obj": obj,
                    "name": None if pname == "-" else pname})
    return out


def label_name(e):
    return e["name"] or e["symbol"]


def parse_markup(path):
    """-> ({addr: (name, line_index)}, lines) for F rows."""
    funcs, lines = {}, []
    with open(path, encoding="utf-8") as f:
        for i, raw in enumerate(f):
            lines.append(raw.rstrip("\n"))
            cols = lines[-1].split("\t")
            if len(cols) >= 3 and cols[0] == "F":
                try:
                    funcs[int(cols[1], 16)] = (cols[2], i)
                except ValueError:
                    pass
    return funcs, lines


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--source", help="URL or local path of psyq_funcs.txt "
                                     "(default: the Wyrelade repo on GitHub)")
    ap.add_argument("--cache", help="keep/reuse the downloaded file here "
                                    "(default: a temp dir, re-downloaded each run)")
    ap.add_argument("--out", help="write labels here instead of stdout")
    ap.add_argument("--format", choices=["ghidra", "tsv"], default="ghidra")
    ap.add_argument("--named-only", action="store_true",
                    help="skip static/unnamed entries ('-' psyq name)")
    ap.add_argument("--markup", default=MARKUP, help="ac1_markup.tsv to compare against")
    ap.add_argument("--merged", help="write a copy of --markup with the PsyQ names "
                                     "applied / added (never the pristine file)")
    ap.add_argument("--verbose", "-v", action="store_true",
                    help="list every differing / new address in the report")
    a = ap.parse_args()

    if a.merged and os.path.abspath(a.merged) == PRISTINE:
        sys.exit("refusing to overwrite ac1_markup_pristine.tsv")

    cache = a.cache or os.path.join(tempfile.gettempdir(), "ac1_psyq_funcs",
                                    "psyq_funcs.txt")
    if not a.cache and os.path.exists(cache):
        os.remove(cache)            # default: always fetch fresh
    text = fetch(a.source, cache)
    entries = parse_psyq(text)
    if not entries:
        sys.exit("no entries parsed -- format of psyq_funcs.txt changed?")
    emit = [e for e in entries if e["name"]] if a.named_only else entries

    # ---- labels -------------------------------------------------------------
    lines = []
    for e in emit:
        n = label_name(e)
        if a.format == "ghidra":
            lines.append(f"{n} 0x{e['addr']:08X} f")
        else:
            lines.append(f"F\t{e['addr']:08x}\t{n}\tunknown\tundefined {n}(void)")
    body = "\n".join(lines) + "\n"
    if a.out:
        with open(a.out, "w", encoding="utf-8", newline="\n") as f:
            f.write(body)
    else:
        sys.stdout.write(body)

    # ---- report against ac1_markup.tsv ------------------------------------
    err = sys.stderr
    n_named = sum(1 for e in entries if e["name"])
    print(f"psyq_funcs.txt: {len(entries)} library functions "
          f"({n_named} with a PsyQ name, {len(entries) - n_named} static/unnamed); "
          f"{len(emit)} labels written", file=err)
    if not os.path.exists(a.markup):
        print(f"(no {a.markup}; comparison skipped)", file=err)
        return
    funcs, mlines = parse_markup(a.markup)
    exe_funcs = {ad: v for ad, v in funcs.items() if EXE_LO <= ad < EXE_HI}

    same, generic, differs, missing, static_hits = [], [], [], [], []
    for e in entries:
        hit = funcs.get(e["addr"])
        if not e["name"]:
            if hit:
                static_hits.append((e, hit[0]))
            else:
                missing.append(e)
            continue
        if hit is None:
            missing.append(e)
        elif hit[0] == e["name"]:
            same.append(e)
        elif GENERIC.match(hit[0]):
            generic.append((e, hit[0]))
        else:
            differs.append((e, hit[0]))
    psyq_addrs = {e["addr"] for e in entries}
    markup_only = sorted(ad for ad in exe_funcs
                         if LIB_LO <= ad < LIB_HI and ad not in psyq_addrs)

    n_missing_named = sum(1 for e in missing if e["name"])
    print(f"ac1_markup.tsv: {len(exe_funcs)} F rows in the exe range "
          f"0x{EXE_LO:08X}-0x{EXE_HI:08X}", file=err)
    print(f"  same name already         : {len(same)}", file=err)
    print(f"  placeholder -> PsyQ name  : {len(generic)}   (gain a name)", file=err)
    print(f"  different name            : {len(differs)}   (corrected / aliased)", file=err)
    print(f"  not in markup (new func)  : {len(missing)}   "
          f"({n_missing_named} named, {len(missing) - n_missing_named} static)", file=err)
    print(f"  static in psyq, named here: {len(static_hits)}   (keep ours)", file=err)
    print(f"  markup-only in lib block  : {len(markup_only)}   "
          f"(0x{LIB_LO:08X}-0x{LIB_HI:08X}, our split, no PsyQ signature)", file=err)
    if a.verbose:
        for e, old in generic:
            print(f"    gain   0x{e['addr']:08X}  {old:<22} -> {e['name']}  "
                  f"[{e['lib']}/{e['obj']}]", file=err)
        for e, old in differs:
            print(f"    differ 0x{e['addr']:08X}  {old:<22} -> {e['name']}  "
                  f"[{e['lib']}/{e['obj']}]", file=err)
        for e in missing:
            print(f"    new    0x{e['addr']:08X}  {label_name(e):<22}    "
                  f"[{e['lib']}/{e['obj']}]", file=err)
        for e, old in static_hits:
            print(f"    static 0x{e['addr']:08X}  {old:<22} (psyq {e['symbol']})  "
                  f"[{e['lib']}/{e['obj']}]", file=err)
        for ad in markup_only:
            print(f"    ours   0x{ad:08X}  {funcs[ad][0]}", file=err)

    # ---- optional merged markup ------------------------------------------
    if a.merged:
        out = list(mlines)
        for e, old in generic + differs:
            i = funcs[e["addr"]][1]
            cols = out[i].split("\t")
            cols[2] = e["name"]
            if len(cols) >= 5:
                cols[4] = re.sub(r"\b" + re.escape(old) + r"\b", e["name"], cols[4], count=1)
            out[i] = "\t".join(cols)
        for e in missing:
            n = label_name(e)
            out.append(f"F\t{e['addr']:08x}\t{n}\tunknown\tundefined {n}(void)")
        with open(a.merged, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(out) + "\n")
        print(f"merged markup written: {a.merged} "
              f"({len(generic) + len(differs)} renamed, {len(missing)} added)", file=err)


if __name__ == "__main__":
    main()
