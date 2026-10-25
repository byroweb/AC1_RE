#!/usr/bin/env python3
"""A wave: a batch of matching agents working the open functions in parallel.

Run from the repo root. OUT is a work folder outside the repo (agents write there, never
into the checkout).

  python tools/loop/wave/wave.py build OUT [--drafts DIR ...] [--agents 3] [--giant 4096]
      One row per FAMILY of open functions (nearport.py families: one twin group or the
      same instruction shape), led by the member with the best draft. Every draft found
      under --drafts DIR (any .../<unit>/func_<ADDR>.c, e.g. earlier agents' scratch
      dirs) is scored with one compile. Writes OUT/all.tsv, OUT/agentN.tsv (dealt
      round-robin, best credit per effort first), OUT/held.tsv (bodies over --giant
      bytes), OUT/ports.tsv (families with a matched near-copy: run nearport.py port
      first), OUT/drafts/, and OUT/agentN/ scratch dirs.
  python tools/loop/wave/wave.py brief OUT
      OUT/BRIEF.md, the shared agent brief, from tools/loop/wave/BRIEF.md.
  python tools/loop/wave/wave.py status OUT
      Rows done / matched per agent, from each OUT/agentN/PROGRESS.md.
  python tools/loop/wave/wave.py items SCRATCH
      Promote items for an agent's clean MATCH rows not yet in re/match.
  python tools/loop/wave/wave.py promote SCRATCH LABEL [ITEM ...]
      Copy each matched C file into re/match (replacing a NOT MATCHED stub), re-run
      drive.py try there (manifest and
      ledger rows), add the agent's names.tsv as hints, and commit. ITEMs default to
      `items SCRATCH`. Then run twins.py replay and drive.py verify.
"""
import argparse
import collections
import glob
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
LOOP = os.path.dirname(HERE)
REPO = os.path.dirname(os.path.dirname(LOOP))
sys.path.insert(0, LOOP)
import drive  # noqa: E402
import nearport  # noqa: E402
import provisional  # noqa: E402
import permute  # noqa: E402

UNIT_RE = r"(?:exe|ovl20[1-4]|fdat\d{3})"
COLS = ["unit", "vaddr", "size", "copies", "words", "source", "opts", "draft", "note", "members"]


# ---------------------------------------------------------------------------
# build
# ---------------------------------------------------------------------------
def progress_lines(dirs):
    """{(unit, vaddr): (note, opts)} from every PROGRESS.md under dirs (last line wins)."""
    out = {}
    for d in dirs:
        for pm in glob.glob(os.path.join(d, "**", "PROGRESS.md"), recursive=True):
            for line in open(pm, encoding="utf-8", errors="replace"):
                m = re.match(r"(%s) (0x[0-9A-Fa-f]+) (\S+)\s*(.*)" % UNIT_RE, line.strip())
                if not m:
                    continue
                opts = {}
                r = re.search(r"--rodata\s+(0x[0-9A-Fa-f]+)", line)
                if r:
                    opts["rodata"] = r.group(1)
                if "--no-rodata-align-fix" in line:
                    opts["noalign"] = True
                out[(m.group(1), int(m.group(2), 16))] = ((m.group(3) + " " + m.group(4)).strip()[:220], opts)
    return out


def drafts_of(dirs, unit, va):
    fn = "func_%08X.c" % va
    out = []
    p = os.path.join(REPO, "re", "match", unit, fn)
    if os.path.isfile(p):                                    # a NOT MATCHED file left in re/match
        out.append(("re/match", p))
    for d in dirs:
        for p in glob.glob(os.path.join(d, "**", unit, fn), recursive=True):
            out.append((os.path.relpath(p, d).split(os.sep)[0], p))
    return out


def score(unit, va, path, opts, work, tools):
    """Words off retail for one draft (one compile), or None if it does not build."""
    try:
        d = os.path.join(work, "%s_%08X" % (unit, va))
        meta = permute.setup_dir(unit, va, path, d, rodata=opts.get("rodata"), tools=tools)
        if opts.get("noalign"):
            meta["no_rodata_align_fix"] = True
        syms = permute.symtab(meta)
        retail = permute.retail_bytes(unit, va, meta["size"])
        text = open(path, encoding="utf-8").read()
        got = permute.text_bytes(meta, syms, text, os.path.dirname(os.path.abspath(path)))
        return None if got is None else permute.word_score(got[0], retail)
    except Exception:  # noqa: BLE001 - a draft that does not build simply scores nothing
        return None


def write_tsv(path, rows):
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\t".join(COLS) + "\n")
        for r in rows:
            fh.write("\t".join(str(r[c]) for c in COLS) + "\n")


def cmd_build(args):
    out = os.path.abspath(args.out)
    dirs = [os.path.abspath(d) for d in args.drafts]
    os.makedirs(os.path.join(out, "drafts"), exist_ok=True)
    notes = progress_lines(dirs)
    sizes = {(u, va): sz for u, va, sz in nearport.inventory()[0]}
    fams = nearport.families(args.min)
    work = tempfile.mkdtemp(prefix="wave_")
    tools = permute.tools_once()
    rows, ports = [], []
    for members, best_port in fams:
        best = None
        for u, va in members:
            opts = notes.get((u, va), ("", {}))[1]
            for kind, p in drafts_of(dirs, u, va):
                w = score(u, va, p, opts, work, tools)
                if w is not None and (best is None or w < best[0]):
                    best = (w, kind, p, u, va)
        u, va = (best[3], best[4]) if best else sorted(members)[0]
        note, opts = notes.get((u, va), ("", {}))
        row = dict(unit=u, vaddr="0x%08X" % va, size=sizes[(u, va)], copies=len(members), words="",
                   source="-", opts="", draft="-", note=note,
                   members=" ".join("%s:0x%08X" % m for m in members if m != (u, va)) or "-")
        if best:
            dst = os.path.join(out, "drafts", u, "func_%08X.c" % va)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copyfile(best[2], dst)
            row.update(words=best[0], source=best[1], draft=dst.replace("\\", "/"),
                       opts=(("--rodata %s " % opts["rodata"]) if opts.get("rodata") else "")
                       + ("--no-rodata-align-fix" if opts.get("noalign") else ""))
            row["opts"] = row["opts"].strip()
        if best_port:
            row["note"] = ("PORT from matched %s 0x%08X (shape %.3f). " % (best_port[1], best_port[2], best_port[0])
                           + row["note"]).strip()
            ports.append(row)
        rows.append(row)
        print("%-8s 0x%08X %5d B x%d  %5s words  %s" % (u, va, row["size"], row["copies"], row["words"], row["source"]))
    shutil.rmtree(work, ignore_errors=True)

    def prio(r):
        return ((r["words"] if r["words"] != "" else 10 ** 6) / r["copies"], -r["copies"], r["size"])
    held = [r for r in rows if r["size"] > args.giant]
    live = sorted([r for r in rows if r["size"] <= args.giant], key=prio)
    write_tsv(os.path.join(out, "all.tsv"), sorted(rows, key=prio))
    write_tsv(os.path.join(out, "held.tsv"), held)
    write_tsv(os.path.join(out, "ports.tsv"), ports)
    chunks = [[] for _ in range(args.agents)]
    for k, r in enumerate(live):                  # round-robin: every agent gets easy ones first
        chunks[k % args.agents].append(r)
    for k, c in enumerate(chunks, 1):
        write_tsv(os.path.join(out, "agent%d.tsv" % k), c)
        os.makedirs(os.path.join(out, "agent%d" % k), exist_ok=True)
        print("agent%d: %d families, %d functions" % (k, len(c), sum(r["copies"] for r in c)))
    print("held (> %d B): %d families, %d functions" % (args.giant, len(held), sum(r["copies"] for r in held)))
    print("ports (matched near-copy, run nearport.py port first): %d" % len(ports))
    # provisional matches (asm-steered C) to rewrite cleanly; the C in re/match is the draft
    rework, seen, prov = [], set(), provisional.compute()
    for unit, va, src, why in prov:
        if src in seen or src.split("/")[0] != unit:          # one row per original C file
            continue
        seen.add(src)
        copies = sum(1 for r in prov if r[2] == src)
        rework.append(dict(unit=unit, vaddr=va, size=sizes.get((unit, int(va, 16)), ""), copies=copies,
                           words=0, source="re/match", opts="", draft="re/match/" + src,
                           note="PROVISIONAL (%s): rewrite without asm pins; the bytes already match" % why,
                           members="-"))
    write_tsv(os.path.join(out, "rework.tsv"), rework)
    print("rework (provisional matches to rewrite without asm): %d" % len(rework))
    return 0


# ---------------------------------------------------------------------------
# brief / status
# ---------------------------------------------------------------------------
def cmd_brief(args):
    out = os.path.abspath(args.out)
    n_open = sum(1 for _ in nearport.inventory()[0])
    text = open(os.path.join(HERE, "BRIEF.md"), encoding="utf-8").read()
    text = text.replace("{REPO}", REPO).replace("{OUT}", out).replace("{OPEN}", str(n_open))
    with open(os.path.join(out, "BRIEF.md"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text)
    print("wrote %s (%d open functions)" % (os.path.join(out, "BRIEF.md"), n_open))
    return 0


def cmd_status(args):
    out = os.path.abspath(args.out)
    for tsv in sorted(glob.glob(os.path.join(out, "agent*.tsv"))):
        name = os.path.basename(tsv)[:-4]
        total = sum(1 for _ in open(tsv, encoding="utf-8")) - 1
        st = collections.Counter()
        pm = os.path.join(out, name, "PROGRESS.md")
        if os.path.isfile(pm):
            for line in open(pm, encoding="utf-8", errors="replace"):
                p = line.split()
                if len(p) > 2 and re.match(UNIT_RE + "$", p[0]):
                    st[p[2]] += 1
        print("%-8s %2d/%-2d done  MATCH %d  NEAR %d  SKIP %d"
              % (name, sum(st.values()), total, st["MATCH"], st["NEAR"], st["SKIP"]))
    return 0


# ---------------------------------------------------------------------------
# items / promote
# ---------------------------------------------------------------------------
def matched_file(path):
    """True when re/match already holds a real match here (a NOT MATCHED stub does not count)."""
    if not os.path.exists(path):
        return False
    with open(path, encoding="utf-8", errors="replace") as fh:
        return "NOT MATCHED" not in fh.read(4096)


def items(scratch):
    """unit:0xVADDR[@rodata=VA][@noalign][@NAME=ADDR...] for clean MATCH rows whose C file is in
    SCRATCH and not yet in re/match. Flags come from the drive.py `commit:` line the agent
    copied into its PROGRESS.md row."""
    seen, out = set(), []
    for ln in open(os.path.join(scratch, "PROGRESS.md"), encoding="utf-8", errors="replace"):
        p = ln.split()
        if len(p) < 3 or p[2] != "MATCH" or not re.match(UNIT_RE + "$", p[0]):
            continue
        v = "%08X" % int(p[1], 16)
        if (p[0], v) in seen or not os.path.exists(os.path.join(scratch, p[0], "func_%s.c" % v)):
            continue
        if matched_file(os.path.join(REPO, "re", "match", p[0], "func_%s.c" % v)):
            continue
        seen.add((p[0], v))
        m = re.search(r"--rodata (0x[0-9A-Fa-f]+)", ln)
        extra = "".join("@" + d for d in re.findall(r"--defsym (\w+=0x[0-9A-Fa-f]+)", ln))
        noalign = "@noalign" if "--no-rodata-align-fix" in ln else ""
        out.append("%s:0x%s%s%s%s" % (p[0], v, "@rodata=" + m.group(1) if m else "", noalign, extra))
    return out


def cmd_items(args):
    print(" ".join(items(args.scratch)))
    return 0


def cmd_promote(args):
    scratch = os.path.abspath(args.scratch)
    todo = args.items or items(scratch)
    ok = []
    for it in todo:
        it, *defs = it.split("@")                      # unit:vaddr@NAME=ADDR@rodata=VA ...
        unit, v = it.split(":")
        v = v.upper().replace("0X", "")
        src = os.path.join(scratch, unit, "func_%s.c" % v)
        dst = os.path.join(REPO, "re", "match", unit, "func_%s.c" % v)
        if matched_file(dst):
            print("SKIP %s 0x%s  already in re/match" % (unit, v))
            continue
        stub = open(dst, encoding="utf-8").read() if os.path.exists(dst) else None   # a NOT MATCHED stub
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copyfile(src, dst)
        extra = []
        for d in defs:
            if d == "noalign":
                extra += ["--no-rodata-align-fix"]
            elif d.startswith("rodata="):
                extra += ["--rodata", d.split("=", 1)[1]]
            else:
                extra += ["--defsym", d]
        r = subprocess.run([sys.executable, os.path.join(LOOP, "drive.py"), "try", unit, "0x" + v] + extra,
                           cwd=REPO, capture_output=True, text=True, errors="replace")
        res = [ln for ln in r.stdout.splitlines() if ln.startswith("result:")]
        if r.returncode == 0:
            ok.append((unit, v))
            why = provisional.reasons(dst)
            print("OK   %s 0x%s  %s%s" % (unit, v, res[0] if res else "", "  PROVISIONAL (%s)" % why if why else ""))
            continue
        if stub is None:                               # leave re/match as it was
            os.remove(dst)
        else:
            with open(dst, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(stub)
        side = dst[:-2] + ".attempts"
        if os.path.exists(side):
            os.remove(side)
        print("FAIL %s 0x%s  %s" % (unit, v, res[0] if res else r.stdout[-300:]))
    nadd = 0
    names = os.path.join(scratch, "names.tsv")
    if os.path.isfile(names):
        for line in open(names, encoding="utf-8"):
            p = line.rstrip("\n").split("\t")
            if len(p) < 4 or p[0] == "unit" or p[2] in ("-", ""):
                continue
            try:
                va = "0x%08X" % int(p[1], 16)
            except ValueError:
                continue
            for kind, val in (("name", p[2]), ("hint", p[3])):
                subprocess.run([sys.executable, os.path.join(LOOP, "hints.py"), "add", p[0], va, kind, val],
                               cwd=REPO, check=False, stdout=subprocess.DEVNULL)
            nadd += 1
    print("names added: %d" % nadd)
    if ok or nadd:
        provisional.main(["scan"])                     # keep re/match/provisional.tsv current
        subprocess.run(["git", "add", "re/match", "tools/loop/ledger.tsv"], cwd=REPO, check=True)
        what = ("matched %d %s functions (%s)" % (len(ok), args.label, ", ".join("%s 0x%s" % k for k in ok))
                if ok else "name proposals for %s functions" % args.label)
        subprocess.run(["git", "commit", "-q", "-m", what + (" + name proposals" if ok and nadd else ""),
                        "-m", "Matched in a scratch dir and re-verified with drive.py try in re/match.",
                        "-m", drive.COMMIT_TRAILER], cwd=REPO, check=True)
        print("committed %d" % len(ok))
    return 0


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("build", help="families -> per-agent work lists with the best draft each")
    s.add_argument("out")
    s.add_argument("--drafts", action="append", default=[], help="folder of earlier drafts (repeatable)")
    s.add_argument("--agents", type=int, default=3)
    s.add_argument("--giant", type=int, default=4096, help="bodies above this many bytes go to held.tsv")
    s.add_argument("--min", type=float, default=0.97, help="shape similarity that makes a family")
    s = sub.add_parser("brief", help="write OUT/BRIEF.md")
    s.add_argument("out")
    s = sub.add_parser("status", help="per-agent progress")
    s.add_argument("out")
    s = sub.add_parser("items", help="promote items from an agent's PROGRESS.md")
    s.add_argument("scratch")
    s = sub.add_parser("promote", help="copy matches into re/match, re-verify, commit")
    s.add_argument("scratch")
    s.add_argument("label")
    s.add_argument("items", nargs="*")
    args = p.parse_args(argv)
    return {"build": cmd_build, "brief": cmd_brief, "status": cmd_status,
            "items": cmd_items, "promote": cmd_promote}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
