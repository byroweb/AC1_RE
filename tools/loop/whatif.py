#!/usr/bin/env python3
"""What-if search: which single compiler decision, flipped, makes a near miss match?

A near miss usually comes down to one decision inside cc1: which pseudo gets a register
first, whether two quantities are tied, which insn the scheduler issues first, whether
cse folds a register back into its constant, whether loop hoists an insn, whether two
tails are cross-jumped. Agents can only nudge those decisions blindly from the C side.
This tool compiles the same C with tools/match/vendor/cc1-whatif.exe, a cc1 built from
the pinned source plus knobs that flip exactly one such decision (tools/match/whatif/),
and reports which knobs bring the function closer to retail.

A what-if result is never a match. It names the decision to provoke from the C side,
for example "pseudo 87 must outrank pseudo 91 in global-alloc" or "insn 142 must issue
before insn 140 in sched2".

  python tools/loop/whatif.py selftest [--n 150]
      With no knobs set, cc1-whatif must produce the pinned cc1's bytes. Compiles N
      matched functions and expects 0 words off for each.
  python tools/loop/whatif.py search UNIT VADDR CFILE [--rodata VA] [--no-rodata-align-fix]
                                     [--window 12] [--top 15] [-j 2] [--json OUT]
      The baseline, then every knob on the pseudos and on the insns near each differing
      word, then a greedy pass that combines the best ones. Prints the knobs that help.

Knobs (environment variables read by cc1-whatif; KEY = pseudo regno or insn uid):
  WHATIF_GPRIO=R:pct   global-alloc priority of pseudo R scaled to pct %
  WHATIF_LPRIO=R:pct   local-alloc priority of the quantity headed by pseudo R
  WHATIF_NOTIE=R       local-alloc never ties pseudo R to another register (combine_regs)
  WHATIF_NOFOLD=R      cse never replaces pseudo R by its constant (equiv_constant)
  WHATIF_SCHED1=U:d    sched1 priority of insn U plus d (sched2: WHATIF_SCHED2)
  WHATIF_NOMOVE=U      loop never hoists insn U
  WHATIF_NOXJUMP=1     no cross-jumping (jump.c)
  WHATIF_MEMDEP=1      the scheduler treats every pair of memory references as dependent
"""
import argparse
import concurrent.futures as cf
import json
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, "tools", "match"))
import match_fn  # noqa: E402
import permute  # noqa: E402
import progress  # noqa: E402

CC1_WHATIF = os.environ.get("AC1_CC1_WHATIF") or os.path.join(REPO, "tools", "match", "vendor",
                                                                 "cc1-whatif.exe")
KNOB_VARS = ("WHATIF_GPRIO", "WHATIF_LPRIO", "WHATIF_NOTIE", "WHATIF_NOFOLD", "WHATIF_SCHED1",
             "WHATIF_SCHED2", "WHATIF_NOMOVE", "WHATIF_NOXJUMP", "WHATIF_MEMDEP")
PSEUDO = re.compile(r"^Register (\d+) used", re.M)
DP = re.compile(r"#\s+(\d+)\s+\S+\s*$")
MOVABLE = re.compile(r"^Insn (\d+): regno", re.M)
WHAT = {
    "WHATIF_GPRIO": "global-alloc priority of pseudo %s x%s%%",
    "WHATIF_LPRIO": "local-alloc priority of pseudo %s's quantity x%s%%",
    "WHATIF_NOTIE": "local-alloc does not tie pseudo %s",
    "WHATIF_NOFOLD": "cse does not fold pseudo %s into its constant",
    "WHATIF_SCHED1": "sched1 priority of insn %s %+d",
    "WHATIF_SCHED2": "sched2 priority of insn %s %+d",
    "WHATIF_NOMOVE": "loop does not hoist insn %s",
    "WHATIF_NOXJUMP": "no cross-jumping",
    "WHATIF_MEMDEP": "scheduler orders every memory pair",
}


class Fn:
    """One function's C, preprocessed once, compiled with cc1-whatif under knob sets."""

    def __init__(self, unit, va, cfile, rodata=None, noalign=False):
        self.unit, self.va = unit, va
        self.tools = dict(permute.tools_once(), cc1=CC1_WHATIF)
        self.base = tempfile.mkdtemp(prefix="whatif_")
        self.meta = permute.setup_dir(unit, va, cfile, os.path.join(self.base, "setup"), rodata=rodata,
                                      tools=self.tools)
        self.meta["tools"] = self.tools
        if noalign:
            self.meta["no_rodata_align_fix"] = True
        self.syms = permute.symtab(self.meta)
        self.retail = permute.retail_bytes(unit, va, self.meta["size"])
        c = os.path.join(self.base, "fn.c")
        shutil.copyfile(cfile, c)
        self.fni = os.path.join(self.base, "fn.i")
        permute.preprocess(self.tools, c, self.fni, [os.path.dirname(os.path.abspath(cfile))])

    def compile(self, knobs, dumps=False):
        """-> (words, info) for one knob set {VAR: 'K:V,...'}; words None on a build error."""
        work = tempfile.mkdtemp(prefix="k_", dir=self.base)
        env = {k: v for k, v in os.environ.items() if k not in KNOB_VARS}
        env.update(knobs)
        try:
            shutil.copyfile(self.fni, os.path.join(work, "fn.i"))
            extra = ["-dp", "-dl", "-dS", "-dR", "-dL"] if dumps else []
            p = subprocess.run(match_fn.cc1_command(self.tools["cc1"]) + ["fn.i"] + match_fn.CC1_FLAGS + extra
                               + ["-o", "fn.s"], cwd=work, env=env, stdin=subprocess.DEVNULL,
                               capture_output=True, text=True)
            if p.returncode != 0:
                return None, {}
            asm = open(os.path.join(work, "fn.s"), encoding="utf-8", errors="replace").read()
            fixed = asm if self.meta.get("no_rodata_align_fix") else match_fn.fix_rodata_align(asm)[0]
            with open(os.path.join(work, "fn.maspsx.s"), "w", newline="\n") as fh:
                fh.write(permute.maspsx_inproc(self.tools["maspsx"], fixed))
            permute.run([self.tools["as"]] + match_fn.AS_FLAGS + ["-o", "fn.o", "fn.maspsx.s"], work)
            blob, _unres = permute.resolve_object(os.path.join(work, "fn.o"), None, self.meta, self.syms)
            info = {"order": order_score(blob, self.retail), "bytes": blob}
            if dumps:
                def dump(ext):
                    f = os.path.join(work, "fn.i." + ext)
                    return open(f, encoding="utf-8", errors="replace").read() if os.path.isfile(f) else ""
                info.update(asm=asm, lreg=dump("lreg"), loop=dump("loop"))
            return permute.word_score(blob, self.retail), info
        except (RuntimeError, match_fn.ToolError, OSError, ValueError):
            return None, {}
        finally:
            shutil.rmtree(work, ignore_errors=True)

    def close(self):
        shutil.rmtree(self.base, ignore_errors=True)


def order_score(ours, retail):
    """Words that differ with the register fields masked: instruction order and shape only
    (a knob that fixes the order but leaves registers renamed scores better here)."""
    n = min(len(ours), len(retail)) // 4
    bad = 0
    for i in range(n):
        a = int.from_bytes(ours[4 * i:4 * i + 4], "little")
        b = int.from_bytes(retail[4 * i:4 * i + 4], "little")
        m = 0xFC00F83F if a >> 26 == 0 else 0xFC00FFFF        # R-type: drop rs/rt/rd; else rs/rt
        if a & m != b & m:
            bad += 1
    return bad + abs(len(ours) - len(retail)) // 2


def diff_words(ours, retail):
    n = min(len(ours), len(retail)) // 4
    return [i for i in range(n) if ours[4 * i:4 * i + 4] != retail[4 * i:4 * i + 4]]


def asm_uids(asm):
    """Insn uid of each instruction line of a -dp listing, in order."""
    out = []
    for line in asm.splitlines():
        s = line.strip()
        if not s or s.startswith((".", "#")) or s.endswith(":"):
            continue
        m = DP.search(line)
        out.append(int(m.group(1)) if m else None)
    return out


def describe(knobs):
    parts = []
    for var, val in sorted(knobs.items()):
        if var in ("WHATIF_NOXJUMP", "WHATIF_MEMDEP"):
            parts.append(WHAT[var])
            continue
        for item in val.split(","):
            k, _, v = item.partition(":")
            if var.startswith("WHATIF_SCHED"):
                parts.append(WHAT[var] % (k, int(v)))
            elif var in ("WHATIF_GPRIO", "WHATIF_LPRIO"):
                parts.append(WHAT[var] % (k, v))
            else:
                parts.append(WHAT[var] % k)
    return "; ".join(parts) or "no knobs"


def merge(a, b):
    out = dict(a)
    for k, v in b.items():
        if k not in out or k in ("WHATIF_NOXJUMP", "WHATIF_MEMDEP"):
            out[k] = v
        else:
            have = out[k].split(",")
            keys = {i.partition(":")[0] for i in have}
            out[k] = ",".join(have + [i for i in v.split(",") if i.partition(":")[0] not in keys])
    return out


def candidates(info, retail, window):
    """Single-knob sets worth trying: every pseudo, and the insns near each differing word."""
    pseudos = sorted({int(m.group(1)) for m in PSEUDO.finditer(info["lreg"])})
    uids = asm_uids(info["asm"])
    near = set()
    for d in diff_words(info["bytes"], retail) or [len(uids) - 1]:
        for i in range(max(0, d - window), min(len(uids), d + window + 1)):
            if uids[i] is not None:
                near.add(uids[i])
    movables = {int(m.group(1)) for m in MOVABLE.finditer(info["loop"])}
    out = [{"WHATIF_NOXJUMP": "1"}, {"WHATIF_MEMDEP": "1"}]
    for r in pseudos:
        out += [{"WHATIF_GPRIO": "%d:300" % r}, {"WHATIF_GPRIO": "%d:30" % r},
                {"WHATIF_LPRIO": "%d:300" % r}, {"WHATIF_LPRIO": "%d:30" % r},
                {"WHATIF_NOTIE": "%d" % r}, {"WHATIF_NOFOLD": "%d" % r}]
    for u in sorted(near):
        out += [{"WHATIF_SCHED1": "%d:15" % u}, {"WHATIF_SCHED1": "%d:-15" % u},
                {"WHATIF_SCHED2": "%d:15" % u}, {"WHATIF_SCHED2": "%d:-15" % u}]
    for u in sorted(movables):
        out.append({"WHATIF_NOMOVE": "%d" % u})
    return out, len(pseudos), len(near), len(movables)


def cmd_search(args):
    va = int(args.vaddr, 16)
    fn = Fn(args.unit, va, args.cfile, args.rodata, args.no_rodata_align_fix)
    try:
        base, plain = fn.compile({})                # the score: a plain compile, like every knob run
        _w, info = fn.compile({}, dumps=True)       # dumps only name the pseudos and insns (-dp
        if base is None or not info:                # comments can change what maspsx emits)
            print("does not build")
            return 2
        info["bytes"], info["order"] = plain["bytes"], plain["order"]
        print("%s 0x%08X: %d words off with no knobs" % (args.unit, va, base))
        if base == 0:
            return 0
        cands, npse, nnear, nmov = candidates(info, fn.retail, args.window)
        print("trying %d knobs: %d pseudos, %d insns near the %d differing words, %d loop movables"
              % (len(cands), npse, nnear, len(diff_words(info["bytes"], fn.retail)), nmov), flush=True)
        border = info["order"]
        results, changed = [], []
        with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
            for knobs, (w, i) in zip(cands, ex.map(fn.compile, cands)):
                if w is None or w == base and i.get("order") == border:
                    continue
                changed.append((w, i["order"], knobs))
                if w < base:
                    results.append((w, knobs))
        results.sort(key=lambda r: r[0])
        print("\n%d knobs change the output, %d help (base: %d words, %d in order/shape):"
              % (len(changed), len(results), base, border))
        for w, k in results[:args.top]:
            print("  %4d words (%+d)  %s" % (w, w - base, describe(k)))
        by_order = sorted((c for c in changed if c[1] < border), key=lambda c: (c[1], c[0]))
        if by_order:
            print("\nknobs that fix order/shape (registers aside):")
            for w, o, k in by_order[:args.top]:
                print("  order %3d (%+d), %4d words  %s" % (o, o - border, w, describe(k)))
        # pairs: an order knob with an allocation knob, since a miss is often both at once
        order_pool = [k for _w, _o, k in by_order[:args.pairs]]
        alloc_pool = [k for _w, _o, k in sorted(changed, key=lambda c: (c[0], c[1]))
                      if set(k) & {"WHATIF_GPRIO", "WHATIF_LPRIO", "WHATIF_NOTIE", "WHATIF_NOFOLD"}][:args.pairs]
        pairs = [merge(a, b) for a in order_pool for b in alloc_pool]
        if pairs:
            print("\ntrying %d order x allocation pairs" % len(pairs), flush=True)
            with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
                for knobs, (w, _i) in zip(pairs, ex.map(fn.compile, pairs)):
                    if w is not None and w < base:
                        results.append((w, knobs))
            results.sort(key=lambda r: r[0])
        # greedy: stack the helpful knobs while they keep helping
        best_w, best_k = (results[0] if results else (base, {}))
        pool = [k for _w, k in results[1:40]] + order_pool[:10]
        improved = True
        while improved and best_w > 0 and pool:
            improved = False
            trials = [merge(best_k, k) for k in pool]
            with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
                scored = [(w, k) for k, (w, _i) in zip(trials, ex.map(fn.compile, trials)) if w is not None]
            scored.sort(key=lambda r: r[0])
            if scored and scored[0][0] < best_w:
                best_w, best_k = scored[0]
                improved = True
                pool = [k for k in pool if not set(k.items()) <= set(best_k.items())]
        if results:
            print("\ncombined: %d words with %s" % (best_w, describe(best_k)))
            print("env: " + " ".join("%s=%s" % kv for kv in sorted(best_k.items())))
            if best_w == 0:
                print("=> these decisions are all that stands between this C and a match")
        if args.json:
            with open(args.json, "w", encoding="utf-8") as fh:
                json.dump({"unit": args.unit, "vaddr": "0x%08X" % va, "base": base,
                           "single": [{"words": w, "knobs": k, "what": describe(k)} for w, k in results],
                           "combined": {"words": best_w, "knobs": best_k, "what": describe(best_k)}}, fh, indent=1)
        return 0
    finally:
        fn.close()


def cmd_selftest(args):
    """No knobs: cc1-whatif must write the same assembly as the pinned cc1, for a sample
    of matched C files (the whole translation unit, every function in it)."""
    tools = permute.tools_once()
    srcs = sorted({line.split("\t")[2] for line in open(os.path.join(REPO, "re", "match", "manifest.tsv"),
                                                          encoding="utf-8")
                   if line.startswith("0x") and line.count("\t") >= 3})
    random.Random(1).shuffle(srcs)
    env = {k: v for k, v in os.environ.items() if k not in KNOB_VARS}
    same = bad = skip = 0
    for src in srcs[:args.n]:
        path = os.path.join(REPO, "re", "match", src)
        work = tempfile.mkdtemp(prefix="wi_self_")
        try:
            permute.preprocess(tools, path, os.path.join(work, "fn.i"), [os.path.dirname(path)])
            outs = []
            for cc1 in (tools["cc1"], CC1_WHATIF):
                p = subprocess.run(match_fn.cc1_command(cc1) + ["fn.i"] + match_fn.CC1_FLAGS + ["-o", "fn.s"],
                                   cwd=work, env=env, stdin=subprocess.DEVNULL, capture_output=True, text=True)
                outs.append(open(os.path.join(work, "fn.s"), encoding="utf-8", errors="replace").read()
                            if p.returncode == 0 else None)
            if outs[0] is None:
                skip += 1
            elif outs[0] == outs[1]:
                same += 1
            else:
                bad += 1
                print("DIFF %s" % src)
        except (RuntimeError, match_fn.ToolError, OSError):
            skip += 1
        finally:
            shutil.rmtree(work, ignore_errors=True)
    print("selftest: %d C files identical, %d differ, %d skipped (do not preprocess alone)" % (same, bad, skip))
    return 1 if bad else 0


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("selftest")
    s.add_argument("--n", type=int, default=150)
    s = sub.add_parser("search")
    s.add_argument("unit")
    s.add_argument("vaddr")
    s.add_argument("cfile")
    s.add_argument("--rodata")
    s.add_argument("--no-rodata-align-fix", action="store_true")
    s.add_argument("--window", type=int, default=12, help="insns either side of a differing word")
    s.add_argument("--top", type=int, default=15)
    s.add_argument("--pairs", type=int, default=20, help="order knobs x allocation knobs to pair up")
    s.add_argument("-j", "--jobs", type=int, default=2)
    s.add_argument("--json")
    args = p.parse_args(argv)
    if not os.path.isfile(CC1_WHATIF):
        sys.exit("no %s: build it with tools/match/whatif/build.sh" % CC1_WHATIF)
    return cmd_search(args) if args.cmd == "search" else cmd_selftest(args)


if __name__ == "__main__":
    sys.exit(main())
