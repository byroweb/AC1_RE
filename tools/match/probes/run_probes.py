#!/usr/bin/env python3
"""run_probes.py - re-verify the GCC 2.7.2 codegen rules behind the match-function skill table.

    python tools/match/probes/run_probes.py            # all probes
    python tools/match/probes/run_probes.py p07 p10    # some (prefix match on the file name)
    python tools/match/probes/run_probes.py -v         # print both disassemblies

Each probes/pNN_*.c holds two variants of one tiny function: variant A (default) and
variant B (-DVARIANT_B).  The header says what the C change does to the code:

    /* probe: <one line claim>
     * defsym: NAME=0xADDR ...          symbols (anything else is masked by match_fn.py)
     * bytes: differ | same | any        B compared against A's bytes by match_fn.py:
     *                                   differ = must MISMATCH, same = must MATCH,
     *                                   any = not checked (e.g. different symbols)
     * a: <regex>   b: <regex>           must occur in that variant's disassembly
     * not-a: <regex>  not-b: <regex>    must not occur
     * count-a: <n> <regex>  count-b: <n> <regex>
     * masked-ok: yes                   masked relocations are expected (e.g. a static's own .bss)
     */

The disassembly is one line, instructions joined by ";" with numeric register names
and tabs replaced by one space, e.g. "lui $2,0x8004;lhu $7,670($2);...".  Every
compile goes through match_fn.py (pinned cc1 + patched maspsx + as), first against a
zero-filled image to obtain the variant's bytes, then B against A's bytes.
Exit 0 only if every selected probe passes.
"""
import os
import re
import subprocess
import sys
import tempfile
import types

HERE = os.path.dirname(os.path.abspath(__file__))
MATCH_DIR = os.path.dirname(HERE)
MATCH_FN = os.path.join(MATCH_DIR, "match_fn.py")
sys.path.insert(0, MATCH_DIR)
import match_fn  # noqa: E402

BASE = 0x80100000


def parse_header(path):
    text = open(path, encoding="utf-8").read()
    m = re.match(r"\s*/\*(.*?)\*/", text, re.S)
    if not m:
        raise SystemExit("%s: no header comment" % path)
    spec = {"claim": "", "defsym": [], "bytes": "differ", "checks": [], "masked_ok": False}
    for raw in m.group(1).splitlines():
        line = raw.strip().lstrip("*").strip()
        key, sep, val = line.partition(":")
        if not sep:
            continue
        key, val = key.strip(), val.strip()
        if key == "probe":
            spec["claim"] = val
        elif key == "defsym":
            spec["defsym"] += val.split()
        elif key == "bytes":
            spec["bytes"] = val
        elif key == "masked-ok":
            spec["masked_ok"] = val.lower() in ("yes", "true", "1")
        elif key in ("a", "b", "not-a", "not-b"):
            spec["checks"].append((key, None, val))
        elif key in ("count-a", "count-b"):
            n, rx = val.split(None, 1)
            spec["checks"].append((key, int(n), rx))
    return spec


def run_match(src, binpath, keep, defsyms, variant_b):
    cmd = [sys.executable, MATCH_FN, "0x%08X" % BASE, src, "--bin", binpath, "--base", "0x%08X" % BASE,
           "--keep", keep, "--fn", "f"]
    for d in defsyms:
        cmd += ["--defsym", d]
    if variant_b:
        cmd += ["-D", "VARIANT_B"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    last = p.stdout.rstrip("\n").split("\n")[-1]
    return p.returncode, last, p.stdout


def disasm(objdump, binpath):
    out = subprocess.run([objdump, "-D", "-b", "binary", "-m", "mips:3000", "-EL", "-M", "reg-names=numeric",
                          "--adjust-vma=0x%08X" % BASE, binpath], capture_output=True, text=True).stdout
    ins = []
    for line in out.splitlines():
        m = re.match(r"^\s*[0-9a-f]+:\s+[0-9a-f]{8}\s+(.*)$", line)
        if m:
            ins.append(re.sub(r"\s+", " ", m.group(1).strip()))
    return ";".join(ins)


def run_probe(path, objdump, zero, tmp, verbose):
    spec = parse_header(path)
    name = os.path.splitext(os.path.basename(path))[0]
    problems = []
    dis = {}
    bins = {}
    for v in ("a", "b"):
        keep = os.path.join(tmp, name + "_" + v)
        rc, last, out = run_match(path, zero, keep, spec["defsym"], v == "b")
        binp = os.path.join(keep, "ours.bin")
        if rc == 2 or not os.path.isfile(binp):
            return name, spec["claim"], ["variant %s did not build: %s" % (v.upper(), last)], dis
        bins[v] = binp
        dis[v] = disasm(objdump, binp)
        if re.search(r"WARNING: [0-9]+ word[(]s[)] masked", out) and not spec["masked_ok"]:
            problems.append("variant %s has masked words (add a defsym)" % v.upper())
    if spec["bytes"] in ("differ", "same"):
        rc, last, out = run_match(path, bins["a"], os.path.join(tmp, name + "_ba"), spec["defsym"], True)
        want = 1 if spec["bytes"] == "differ" else 0
        if rc != want:
            problems.append("B vs A bytes: expected %s, match_fn said %s" %
                            ("MISMATCH" if want else "MATCH", last))
    for key, n, rx in spec["checks"]:
        v = key[-1]
        hits = len(re.findall(rx, dis[v]))
        if key in ("a", "b") and not hits:
            problems.append("variant %s lacks /%s/" % (v.upper(), rx))
        elif key.startswith("not-") and hits:
            problems.append("variant %s has /%s/" % (v.upper(), rx))
        elif key.startswith("count-") and hits != n:
            problems.append("variant %s has %d x /%s/, expected %d" % (v.upper(), hits, rx, n))
    return name, spec["claim"], problems, dis


def main(argv):
    verbose = "-v" in argv
    sel = [a for a in argv if not a.startswith("-")]
    tc = match_fn.load_toolchain_json()
    try:
        as_ = match_fn.find_as(types.SimpleNamespace(as_=None), tc)
    except match_fn.ToolError as e:
        print("ERROR: %s" % e)
        return 2
    objdump = match_fn.find_objdump(as_)
    files = sorted(f for f in os.listdir(HERE) if re.match(r"p\d\d_.*\.c$", f))
    if sel:
        files = [f for f in files if any(f.startswith(s) for s in sel)]
    fails = 0
    with tempfile.TemporaryDirectory(prefix="probes_") as tmp:
        zero = os.path.join(tmp, "zero.bin")
        open(zero, "wb").write(b"\0" * 0x2000)
        for f in files:
            name, claim, problems, dis = run_probe(os.path.join(HERE, f), objdump, zero, tmp, verbose)
            print("%-4s %-28s %s" % ("ok" if not problems else "FAIL", name, claim))
            for p in problems:
                print("        - " + p)
            if verbose or problems:
                for v in ("a", "b"):
                    if v in dis:
                        print("        %s: %s" % (v.upper(), dis[v]))
            fails += bool(problems)
    print("%d/%d probes hold" % (len(files) - fails, len(files)))
    return 0 if fails == 0 else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
