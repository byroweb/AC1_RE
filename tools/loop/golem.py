#!/usr/bin/env python3
"""golem.py - a local LLM in a closed loop with the oracle: compile fixes and near-miss tweaks.

    golem.py fix   UNIT VADDR FILE.c [--model M] [--attempts 3] [--out F]
    golem.py tweak UNIT VADDR FILE.c [--model M] [--rounds 3] [-n 6] [--out F]
    golem.py run   (--report F ... | --cfile [UNIT:]F ...) [--job auto|fix|tweak] [--max-words 12]
                   [--max-size 800] [--limit N] [--no-commit] [--no-ledger]
    golem.py bench BENCH.tsv --models A,B [--out DIR]

A model served by LM Studio (or anything else with an OpenAI-compatible
/v1/chat/completions; --url, $GOLEM_URL, default http://localhost:1234/v1; --model,
$GOLEM_MODEL) gets one small, strictly formatted job at a time, and every answer is
scored with exactly the oracle's compile (tools/loop/permute.py's in-process pipeline,
tools/match/variants.py's batch scoring: differing words + 2 per word of size
difference, 0 = the retail bytes).  A score-0 text goes through match_fn.py itself;
only a clean MATCH counts.

  fix    a draft cc1 rejects.  The model gets the cc1 errors (mapped back to the
         draft's lines), the prototypes the headers give for the names the code uses,
         and the code after the #include lines; it answers with the whole code in one
         ```c block, which replaces it (or only the function, when the answer holds
         nothing else).  Up to --attempts answers, each from the last one's errors.
  tweak  a draft that compiles but is some words off.  The model gets the code and the
         retail/ours disassembly around the differences (whole listing for short
         functions) and answers with up to -n `## label` / `-old` / `+new` edit blocks
         (variants.py --edits format).  All of them, and pairs of the ones that
         improved, are compiled in one parallel batch; the best improvement becomes the
         next round's input.  --rounds rounds, the edits that did not help are listed
         in the next prompt.

Prompts are kept under --max-prompt characters (~4K tokens): long functions get only
the differing region of the listing; a function whose code alone is over the limit is
skipped.  Qwen3 gets `/no_think`, and any <think> block is stripped.  Every model call
is a row of --log (default ~/dev/ac1_work/golem_log.tsv, as overnight.sh's work dirs;
bench: <out>/golem_log.tsv): job, model, tokens, seconds, score before/after.

run: candidates from sweep_report.tsv (best MISMATCH try -> tweak when within
--max-words, all tries cc1 errors -> fix on the first variant's draft), from
permute_report.tsv (its lever.c / draft.c -> tweak) and --cfile files; bodies matched
in the ledger or with a re/match C file are skipped (except a --cfile).  A MATCH is
written to re/match/<unit>/func_<ADDR>.c, re-verified from there, and gets its
manifest and ledger rows (`golem (<model>, <job>)`), unless --no-commit.

bench: one row per job (tab-separated, header: tier job unit vaddr cfile, optional
size base perm perm_note), every model on every row with the same budget; writes
<out>/bench_results.tsv and prints a per-model table.  Nothing is committed.
"""
import argparse
import datetime
import difflib
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
import types
import urllib.error
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
MATCH_DIR = os.path.join(REPO, "tools", "match")
sys.path.insert(0, HERE)
sys.path.insert(0, MATCH_DIR)
import drive     # noqa: E402
import ledger    # noqa: E402
import m2cfix    # noqa: E402
import match_fn  # noqa: E402
import permute   # noqa: E402
import sweep     # noqa: E402
import units     # noqa: E402
import variants  # noqa: E402

DEFAULT_URL = os.environ.get("GOLEM_URL", "http://localhost:1234/v1")
DEFAULT_MODEL = os.environ.get("GOLEM_MODEL", "qwen2.5-coder-7b-instruct")
DEFAULT_LOG = os.path.join(os.path.expanduser("~"), "dev", "ac1_work", "golem_log.tsv")   # as overnight.sh
MAX_PROMPT = 14000          # characters, ~4K tokens of C and asm
SMALL_FN = 64               # instructions: show the whole listing up to this size
CONTEXT = 5                 # listing rows around each difference otherwise
MAX_ROWS = 90               # listing rows at most
THINK = re.compile(r"<think>.*?(?:</think>|$)", re.S)
FENCE = re.compile(r"```[ \t]*(?:c|C|cpp|diff)?[ \t]*\n(.*?)(?:```|$)", re.S)
INCLUDE = re.compile(r"^\s*#\s*include\b.*$", re.M)
CALLEE = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
GLOBAL = re.compile(r"\b((?:D|g)_\w+)\b")
C_WORDS = {"if", "while", "for", "switch", "return", "sizeof", "do", "else", "case", "goto",
           "M2C_FIELD", "M2C_BITWISE"}
LOG_COLS = ["when", "job", "unit", "vaddr", "size", "model", "round", "prompt_tokens",
            "completion_tokens", "llm_s", "eval_s", "proposals", "built", "before", "after",
            "status", "note"]


class LLMError(Exception):
    pass


# ---------------------------------------------------------------------------
# the model
# ---------------------------------------------------------------------------
def chat(url, model, system, user, max_tokens, temperature=0.2, timeout=600):
    """One chat completion -> {text, prompt_tokens, completion_tokens, seconds}."""
    if "qwen3" in model.lower():
        user += "\n/no_think"
    body = {"model": model, "temperature": temperature, "max_tokens": max_tokens, "stream": False,
            "messages": [{"role": "system", "content": system}, {"role": "user", "content": user}]}
    req = urllib.request.Request(url.rstrip("/") + "/chat/completions", data=json.dumps(body).encode(),
                                 headers={"Content-Type": "application/json"})
    t0 = time.time()
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            data = json.loads(r.read().decode("utf-8", "replace"))
    except (urllib.error.URLError, OSError, ValueError) as e:
        raise LLMError("%s: %s" % (type(e).__name__, e))
    try:
        text = data["choices"][0]["message"].get("content") or ""
    except (KeyError, IndexError, TypeError):
        raise LLMError("unexpected reply: %s" % str(data)[:200])
    usage = data.get("usage") or {}
    return {"text": THINK.sub("", text).strip(), "prompt_tokens": usage.get("prompt_tokens"),
            "completion_tokens": usage.get("completion_tokens"), "seconds": time.time() - t0}


# ---------------------------------------------------------------------------
# the function under work: retail bytes, oracle compile, listing
# ---------------------------------------------------------------------------
class Target:
    def __init__(self, unit, vaddr, cfile, size=None, rodata=None, defsyms=()):
        self.unit, self.vaddr, self.vs = unit, vaddr, "0x%08X" % vaddr
        self.cfile = os.path.abspath(cfile)
        row = drive.load_rows().get((unit, self.vs))
        self.size = size or (int(row["size"]) if row else None)
        if not self.size:
            raise SystemExit("golem.py: %s %s not in functions.tsv: pass the size" % (unit, self.vs))
        if rodata is None and row and int(row.get("jump_tables") or 0) and row.get("jt_addrs"):
            rodata = row["jt_addrs"].split(",")[0].strip()
        self.fn = drive.fn_name(self.vs)
        with open(self.cfile, encoding="utf-8") as fh:
            self.text = fh.read()
        self.meta = {"unit": unit, "vaddr": self.vs, "size": self.size, "rodata": rodata,
                     "fn": self.fn if permute.find_function(self.text, self.fn) else None,
                     "syms": permute.SYMS_TABLES if drive.uses_ac1_h(self.cfile) else [],
                     "defsyms": list(defsyms), "tools": permute.tools_once()}
        self.syms = permute.symtab(self.meta)
        self.retail = permute.retail_bytes(unit, vaddr, self.size)
        self.inc = [os.path.dirname(self.cfile)]
        self._retail_asm = None

    def evaluate(self, text):
        """C text -> {status built|ERROR, score, size, bytes, errors [(line, msg)], error, decls}."""
        work = tempfile.mkdtemp(prefix="golem_")
        tools = self.meta["tools"]
        try:
            c, fn_i = os.path.join(work, "fn.c"), os.path.join(work, "fn.i")
            with open(c, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(text)
            try:
                permute.preprocess(tools, c, fn_i, self.inc)
                ours, unres = permute.resolve_object(permute.build_object(self.meta, fn_i, work, tools),
                                                     None, self.meta, self.syms)
            except (RuntimeError, match_fn.ToolError, OSError, ValueError) as e:
                return self._error(text, work, fn_i, str(e))
            return {"status": "built", "score": permute.word_score(ours, self.retail),
                    "unaligned": variants.unaligned(ours, self.retail), "size": len(ours), "bytes": ours,
                    "unresolved": unres}
        finally:
            shutil.rmtree(work, ignore_errors=True)

    def _error(self, text, work, fn_i, msg):
        out = {"status": "ERROR", "score": None, "errors": [], "decls": []}
        if os.path.exists(fn_i):
            p = subprocess.run(match_fn.cc1_command(self.meta["tools"]["cc1"]) + ["fn.i"] + match_fn.CC1_FLAGS
                               + ["-o", "fn.s"], cwd=work, capture_output=True, text=True,
                               stdin=subprocess.DEVNULL)
            if p.returncode != 0:
                errs = m2cfix.cc1_errors(p.stderr + p.stdout)
                out["errors"] = m2cfix.map_errors(text, fn_i, errs)
                if not out["errors"] and errs:         # in a header: keep cc1's own line numbers
                    out["errors"] = [(None, m) for _ln, m in errs]
                with open(fn_i, encoding="utf-8", errors="replace") as fh:
                    out["decls"] = header_decls(fh.read(), text)
        lines = [ln for ln in msg.replace("\r", "").splitlines() if ln.strip()]
        out["error"] = (out["errors"][0][1] if out["errors"] else
                        " ".join((next((ln for ln in lines if "error" in ln.lower()), lines[0] if lines else
                                       "build failed")).split()))[:160]
        return out

    def disasm(self, blob):
        work = tempfile.mkdtemp(prefix="golem_dis_")
        try:
            d = match_fn.disasm(self.meta["tools"].get("objdump"), blob, self.vaddr, work, "fn")
            return [d.get(self.vaddr + i, "?") for i in range(0, len(blob) - 3, 4)]
        finally:
            shutil.rmtree(work, ignore_errors=True)

    def target_asm(self):
        if self._retail_asm is None:
            self._retail_asm = self.disasm(self.retail)
        return "\n".join("%08x  %s" % (self.vaddr + 4 * i, s) for i, s in enumerate(self._retail_asm))

    def listing(self, ours):
        """Retail vs ours, aligned rows: (mark, addr, retail, ours); mark ! - + or blank."""
        if self._retail_asm is None:
            self._retail_asm = self.disasm(self.retail)
        mine = self.disasm(ours)
        ret = self._retail_asm
        rows = []
        if len(mine) == len(ret):                # same size: position by position
            for i, (a, b) in enumerate(zip(ret, mine)):
                rows.append(("!" if self.retail[4 * i:4 * i + 4] != ours[4 * i:4 * i + 4] else " ",
                             self.vaddr + 4 * i, a, b))
            return rows
        sm = difflib.SequenceMatcher(None, [norm_ins(x) for x in ret], [norm_ins(x) for x in mine],
                                     autojunk=False)
        for op, i1, i2, j1, j2 in sm.get_opcodes():
            if op == "equal":
                rows += [(" ", self.vaddr + 4 * (i1 + k), ret[i1 + k], mine[j1 + k]) for k in range(i2 - i1)]
                continue
            n = max(i2 - i1, j2 - j1)
            for k in range(n):
                a = ret[i1 + k] if i1 + k < i2 else None
                b = mine[j1 + k] if j1 + k < j2 else None
                mark = "!" if a is not None and b is not None else ("-" if b is None else "+")
                rows.append((mark, self.vaddr + 4 * (i1 + k) if a is not None else None, a or "", b or ""))
        return rows


def norm_ins(s):
    """Branch/jump targets differ whenever the sizes do: compare without them."""
    return re.sub(r"0x8[0-9a-f]{7}$", "L", s)


def show_listing(rows, small=SMALL_FN, context=CONTEXT, max_rows=MAX_ROWS):
    """The listing as prompt text: everything for short functions, else the differing
    regions with `context` rows around them, at most max_rows rows."""
    diff = [i for i, r in enumerate(rows) if r[0] != " "]
    keep = set(range(len(rows))) if len(rows) <= small else \
        {k for i in diff for k in range(i - context, i + context + 1) if 0 <= k < len(rows)}
    out = ["  %-8s  %-30s | %s" % ("addr", "target", "ours")]
    last, shown, cut = None, 0, 0
    for i in sorted(keep):
        if shown >= max_rows:
            cut += 1 if rows[i][0] != " " else 0
            continue
        if last is not None and i != last + 1:
            out.append("  ...")
        last = i
        mark, addr, a, b = rows[i]
        out.append("%s %-8s  %-30s | %s" % (mark, "%08x" % addr if addr else "", a, b))
        shown += 1
    if cut:
        out.append("  ... %d more differing row(s) not shown" % cut)
    return "\n".join(out)


def header_decls(fn_i, text, limit=30):
    """Prototypes / extern lines of the preprocessed headers for the names `text` uses
    (fn.i up to where the draft's own lines start: they are its tail, m2cfix.map_errors)."""
    code = code_part(text)
    names = {m.group(1) for m in CALLEE.finditer(code)} - C_WORDS
    names |= {m.group(1) for m in GLOBAL.finditer(code)}
    lines = [ln for ln in fn_i.split("\n") if ln.strip()]
    lines = lines[:max(0, len(lines) - len(m2cfix.code_lines(text)))]
    out = []
    for ln in lines:
        s = ln.strip()
        if not s.endswith(";") or len(s) > 200 or "{" in s:
            continue
        m = re.search(r"\b([A-Za-z_]\w*)\s*(?:\(|\[|;)", s)
        hit = [n for n in names if re.search(r"\b%s\b" % re.escape(n), s)]
        if hit and m and (s.startswith("extern") or "(" in s):
            out.append(s)
            if len(out) >= limit:
                break
    return out


# ---------------------------------------------------------------------------
# text helpers
# ---------------------------------------------------------------------------
def split_code(text):
    """(head, code): head = everything up to the last #include line, code = the rest."""
    last = None
    for m in INCLUDE.finditer(text):
        last = m
    if not last:
        return "", text
    return text[:last.end() + 1], text[last.end() + 1:]


def code_part(text):
    return split_code(text)[1]


def fenced(reply):
    """The code of a reply: the longest fenced block, else the whole reply."""
    blocks = FENCE.findall(reply)
    return max(blocks, key=len) if blocks else reply


def apply_whole(text, reply, fn):
    """The model's code in place of ours: all of the code part when the answer has more
    than the function, else just the function.  None when the function is missing."""
    code = fenced(reply).strip("\n") + "\n"
    span = permute.find_function(code, fn)
    if not span:
        return None
    head, _ = split_code(text)
    if code[:span[0]].strip():
        code = INCLUDE.sub("", code).lstrip("\n")      # its own #includes would come twice
        return head + "\n" + code
    return permute.splice(text, code[span[0]:span[1]], fn)


def parse_edit_reply(reply):
    """`## label` / `-old` / `+new` blocks from a reply, tolerant of chatter: every other
    line is dropped, a block that does not parse is skipped.  -> [(label, hunks)]."""
    body = "\n".join(FENCE.findall(reply)) or reply
    keep = [ln.rstrip("\r") for ln in body.split("\n") if ln.startswith(("##", "-", "+"))]
    out, cur = [], []
    for ln in keep + ["## end"]:
        if ln.startswith("##") and cur:
            try:
                out += variants.parse_edits("\n".join(cur))
            except variants.EditError:
                pass
            cur = []
        if ln.startswith("---") or ln.startswith("+++"):
            continue
        cur.append(ln)
    return [(lbl, hunks) for lbl, hunks in out if any(o.strip() for o, _n in hunks)
            and not all(ASM_LINE.match(ln) for o, _n in hunks for ln in o.split("\n") if ln.strip())]


ASM_LINE = re.compile(r"^\s*[a-z][a-z0-9.]{0,6}\s+(?:\$?\w+|-?\d+\(\w+\))(?:\s*,\s*(?:\$?\w+|-?\w+\(\w+\)))*\s*$"
                      r"|^\s*(?:nop|syscall)\s*$")


# ---------------------------------------------------------------------------
# prompts
# ---------------------------------------------------------------------------
SYSTEM_FIX = """You fix C written by the m2c decompiler so that GCC 2.7.2 (old C89) compiles it.
Keep what the code computes and the order of its statements exactly as they are; change only what the errors need.
Names: M2C_UNK is s32; M2C_FIELD(expr, T *, off) is the T at byte offset off from expr; s8/u8/s16/u16/s32/u32 are the integer types; func_XXXXXXXX and D_XXXXXXXX are the function and the global at that address.
Usual fixes:
- void value not ignored: the header declares the callee void but the code uses its value: call it as ((s32 (*)())func_X)(args)
- too few / too many arguments: pass what the prototype wants (the target assembly shows what the call puts in a0-a3), or call through ((s32 (*)())func_X)(args)
- invalid type argument of unary *: the operand is not a pointer: *(s32 *)(x) (pick the right width)
- undeclared name: declare it as a local at the top of the function
- invalid lvalue in assignment: assign through a pointer, *(T *)&x = v
- structure has no member: use M2C_FIELD(ptr, T *, offset)
Reply with the complete fixed code in one ```c block and nothing else."""

SYSTEM_TWEAK = """You do matching decompilation: PlayStation 1 code compiled by GCC 2.7.2 -O2 for the MIPS R3000. Our C compiles to almost the target instructions; find source edits that make it emit exactly the target.
What changes GCC 2.7.2 output without changing what the code does:
- statement order; the order local variables are declared or first used (it decides registers)
- adding a temporary variable, or removing one
- signedness and width of types and casts (lh/lhu, lb/lbu, sra/srl, andi 0xffff, sll+sra pairs)
- operand order of + * & | ^ ==; a < b versus b > a; x != 0 versus x
- p[i] versus *(p + i); pointer types (scaled index: sll 2 is s32 *, sll 1 is s16 *)
- constants: lui/ori/addiu immediates are the halves of the constant the C must spell
- for / while / do-while / goto forms of one loop; if/else versus ?:; early return
Answer only with edit blocks, like this:
## make the count unsigned
-    s32 n;
+    u32 n;
## swap the two stores
-    a->x = 1;
-    a->y = 2;
+    a->y = 2;
+    a->x = 1;
Each ## block is tried on its own.  The - lines are whole lines copied exactly from our C."""


def fix_prompt(tgt, text, ev, max_prompt=MAX_PROMPT):
    code = code_part(text)
    lines = text.split("\n")
    errs = []
    for ln, msg in ev["errors"][:12]:
        src = lines[ln].strip() if ln is not None and 0 <= ln < len(lines) else "(in a header)"
        errs.append("%s\n    -> %s" % (src, msg))
    decls = "\n".join(ev.get("decls") or []) or "(none found)"
    user = ("GCC errors, each under the line it is about:\n%s\n\n"
            "Declarations from the headers for the names the code uses:\n%s\n\n"
            % ("\n".join(errs) or ev.get("error") or "(none)", decls))
    asm = tgt.target_asm()
    if len(user) + len(code) + len(asm) + 600 < max_prompt:   # what the calls really pass
        user += "The target assembly (arguments go in a0-a3 before each jal):\n%s\n\n" % asm
    user += ("The code (%s, %s, %d bytes of MIPS):\n```c\n%s```\n"
             "Fix the errors (do not answer with the code unchanged).\n" % (tgt.fn, tgt.unit, tgt.size, code))
    return user


LUI = re.compile(r"^lui (\w+),(0x[0-9a-f]+|\d+)$")
LOW = re.compile(r"^(?:addiu|ori) (\w+),(\w+),(-?(?:0x[0-9a-f]+|\d+))$")
LI = re.compile(r"^(?:addiu|ori) (\w+),zero,(-?(?:0x[0-9a-f]+|\d+))$")


def constant_at(rows, i, side):
    """The constant a lui (+ addiu/ori on the same register within 4 rows) or a li builds."""
    ins = rows[i][2 + side]
    m = LI.match(ins)
    if m:
        return int(m.group(2), 0)
    m = LUI.match(ins)
    if not m:
        return None
    val = int(m.group(2), 0) << 16
    for r in rows[i + 1:i + 5]:
        lo = LOW.match(r[2 + side])
        if lo and lo.group(2) == m.group(1):
            k = int(lo.group(3), 0)
            return val + k if r[2 + side].startswith("addiu") else val | k
    return val


def hint(rows, size, ours_size):
    """One line on what the differences look like: drive.pick_hint's rules, and the two
    constants when a lui / li immediate is what differs."""
    for i, (mark, _addr, a, b) in enumerate(rows):
        if mark == "!" and a.split()[:1] == b.split()[:1] and (LUI.match(a) or LI.match(a)):
            ca, cb = constant_at(rows, i, 0), constant_at(rows, i, 1)
            if ca is not None and cb is not None and ca != cb:
                return ("a constant differs: the target builds 0x%X (%d) where ours builds 0x%X (%d); "
                        "spell it in the C the way the target has it." % (ca & 0xFFFFFFFF, ca,
                                                                          cb & 0xFFFFFFFF, cb))
    if any(r[0] in "-+" for r in rows):
        text = ""
    else:
        text = "\n".join("%s %08x  %s | %s" % r for r in rows)
    first = next((i * 4 for i, r in enumerate(rows) if r[0] != " "), None)
    facts = {"result": "MISMATCH", "bytes": ours_size, "first_diff_offset": first, "resolved_symbols": []}
    h = drive.pick_hint(1, facts, text, size)
    return h.split(" - see the diff-pattern table")[0] if h else None


def tweak_prompt(tgt, text, ev, n, tried, max_prompt):
    code = code_part(text)
    span = permute.find_function(code, tgt.fn)
    if len(code) > max_prompt * 0.6 and span:
        code = code[span[0]:span[1]] + "\n"       # the declarations above it do not fit
    rows = tgt.listing(ev["bytes"])
    budget = max(30, int((max_prompt - len(code) - 1500) / 60))
    asm = show_listing(rows, max_rows=min(MAX_ROWS, budget))
    h = hint(rows, tgt.size, ev["size"])
    user = ("Target vs our GCC output (`!` differs, `-` only in the target, `+` only in ours):\n%s\n"
            "%d differing word(s); target %d bytes, ours %d.\n%s\n"
            "Our C (%s, %s):\n```c\n%s```\n"
            % (asm, sum(1 for r in rows if r[0] != " "), tgt.size, ev["size"],
               ("Hint: %s\n" % h) if h else "", tgt.fn, tgt.unit, code))
    if tried:
        user += "\nEdits already tried, no better:\n%s\n" % "\n".join("- " + t for t in tried[-12:])
    user += ("\nEdit the C (never the assembly) so it compiles to the target.  Give up to %d different "
             "edits as ## blocks: every - line is a whole line copied exactly from the C above, the + "
             "lines are its new C.  Nothing else." % n)
    return user


# ---------------------------------------------------------------------------
# jobs
# ---------------------------------------------------------------------------
class Log:
    def __init__(self, path):
        self.path = path
        if path:
            os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
            if not os.path.exists(path):
                with open(path, "w", encoding="utf-8", newline="\n") as fh:
                    fh.write("\t".join(LOG_COLS) + "\n")

    def row(self, **kw):
        if not self.path:
            return
        kw.setdefault("when", datetime.datetime.now().isoformat(timespec="seconds"))
        with open(self.path, "a", encoding="utf-8", newline="\n") as fh:
            fh.write("\t".join(" ".join(str("" if kw.get(c) is None else kw.get(c)).split())
                               for c in LOG_COLS) + "\n")


def oracle_ok(tgt, text, work):
    """match_fn.py on a score-0 text (variants.confirm: auto --defsym, -I the file's dir)."""
    row = {"i": 0, "score": 0, "text": text}
    ns = types.SimpleNamespace(cfile=tgt.cfile, vaddr=tgt.vaddr)
    clean = variants.confirm([row], ns, tgt.meta, tgt.unit, work)
    return bool(clean), row.get("defsyms") or [], row.get("oracle")


def result(tgt, job, model, text, before, ev, calls, t0, **kw):
    out = {"job": job, "unit": tgt.unit, "vaddr": tgt.vs, "size": tgt.size, "model": model,
           "before": before, "after": ev.get("score"), "built": ev["status"] == "built",
           "status": "no match", "text": text, "defsyms": [], "calls": calls["n"],
           "prompt_tokens": calls["pt"], "completion_tokens": calls["ct"], "llm_s": calls["s"],
           "wall_s": time.time() - t0, "note": ""}
    out.update(kw)
    return out


def ask(args, model, system, user, max_tokens, temperature, calls):
    r = chat(args.url, model, system, user, max_tokens, temperature, args.timeout)
    if args.dump:
        os.makedirs(args.dump, exist_ok=True)
        p = os.path.join(args.dump, "%s_%03d" % (model.replace("/", "_"), len(os.listdir(args.dump)) // 2))
        with open(p + "_prompt.txt", "w", encoding="utf-8", newline="\n") as fh:
            fh.write(system + "\n\n=== user ===\n" + user)
        with open(p + "_reply.txt", "w", encoding="utf-8", newline="\n") as fh:
            fh.write(r["text"])
    calls["n"] += 1
    calls["pt"] += r["prompt_tokens"] or 0
    calls["ct"] += r["completion_tokens"] or 0
    calls["s"] += r["seconds"]
    return r


def do_fix(tgt, args, model, log, work):
    """Up to args.attempts whole-code answers, each from the last one's cc1 errors."""
    t0 = time.time()
    calls = {"n": 0, "pt": 0, "ct": 0, "s": 0.0}
    text = tgt.text
    ev = tgt.evaluate(text)
    if ev["status"] == "built":
        return result(tgt, "fix", model, text, "built", ev, calls, t0, status="skipped",
                      note="already compiles")
    code = code_part(text)
    if len(code) > args.max_prompt * 0.75:
        return result(tgt, "fix", model, text, "ERROR", ev, calls, t0, status="skipped",
                      note="code too long for the prompt (%d chars)" % len(code))
    best, best_ev = text, ev
    for k in range(1, args.attempts + 1):
        user = fix_prompt(tgt, best, best_ev, args.max_prompt)
        try:
            r = ask(args, model, SYSTEM_FIX, user, min(4096, 400 + len(code) // 2), 0.2 + 0.2 * (k - 1), calls)
        except LLMError as e:
            log.row(job="fix", unit=tgt.unit, vaddr=tgt.vs, size=tgt.size, model=model, round=k,
                    status="llm error", note=str(e))
            break
        t1 = time.time()
        cand = apply_whole(best, r["text"], tgt.fn)
        if cand is None:
            cev = {"status": "NOREPLY", "score": None, "errors": [],
                   "error": "no definition of %s in the answer" % tgt.fn}
        elif code_part(cand).split() == code_part(best).split():
            cev = dict(best_ev, status="UNCHANGED", error="answer is the code unchanged")
        else:
            cev = tgt.evaluate(cand)
        log.row(job="fix", unit=tgt.unit, vaddr=tgt.vs, size=tgt.size, model=model, round=k,
                prompt_tokens=r["prompt_tokens"], completion_tokens=r["completion_tokens"],
                llm_s="%.1f" % r["seconds"], eval_s="%.1f" % (time.time() - t1), proposals=1,
                built=int(cev["status"] == "built"), before="ERROR(%d)" % len(best_ev["errors"]),
                after=cev["score"] if cev["status"] == "built" else "%s(%d)" % (cev["status"],
                                                                                len(cev.get("errors") or [])),
                status=cev["status"], note=cev.get("error", ""))
        if cev["status"] == "built":
            if cev["score"] == 0:
                ok, defsyms, summ = oracle_ok(tgt, cand, work)
                if ok:
                    return result(tgt, "fix", model, cand, "ERROR", cev, calls, t0, status="MATCH",
                                  defsyms=defsyms)
            return result(tgt, "fix", model, cand, "ERROR", cev, calls, t0, status="compiled")
        if cand and cev["status"] == "ERROR" and len(cev["errors"]) <= len(best_ev["errors"]) and cev["errors"]:
            best, best_ev = cand, cev
    return result(tgt, "fix", model, best, "ERROR", best_ev, calls, t0, status="error",
                  note=best_ev.get("error", ""))


def do_tweak(tgt, args, model, log, work, rounds=None):
    """args.rounds rounds of up to args.n edit blocks, scored in one variants.py batch each."""
    t0 = time.time()
    calls = {"n": 0, "pt": 0, "ct": 0, "s": 0.0}
    text = tgt.text
    ev = tgt.evaluate(text)
    if ev["status"] != "built":
        return result(tgt, "tweak", model, text, None, ev, calls, t0, status="skipped",
                      note="does not compile: " + ev.get("error", ""))
    before = ev["score"]
    if before == 0:
        ok, defsyms, summ = oracle_ok(tgt, text, work)
        if ok:
            return result(tgt, "tweak", model, text, 0, ev, calls, t0, status="MATCH", defsyms=defsyms,
                          note="input already matches")
    span = permute.find_function(text, tgt.fn)
    if (span[1] - span[0] if span else len(code_part(text))) > args.max_prompt * 0.6:
        return result(tgt, "tweak", model, text, before, ev, calls, t0, status="skipped",
                      note="function too long for the prompt")
    tried = []
    seen = {hashlib.sha1(text.encode()).digest()}
    for k in range(1, (rounds or args.rounds) + 1):
        user = tweak_prompt(tgt, text, ev, args.n, tried, args.max_prompt)
        try:
            r = ask(args, model, SYSTEM_TWEAK, user, 120 + 160 * args.n, 0.3 + 0.25 * (k - 1), calls)
        except LLMError as e:
            log.row(job="tweak", unit=tgt.unit, vaddr=tgt.vs, size=tgt.size, model=model, round=k,
                    status="llm error", note=str(e))
            break
        t1 = time.time()
        edits = parse_edit_reply(r["text"])[:args.n * 2]
        cands = [(lbl, "edit", hunks, None) for lbl, hunks in edits]
        rows = variants.materialise(cands, text)
        rows = [x for x in rows if "text" not in x or hashlib.sha1(x["text"].encode()).digest() not in seen]
        variants.run_batch(rows, tgt.meta, tgt.syms, tgt.retail, tgt.inc, args.jobs, False)
        better = sorted([x for x in rows if x.get("score") is not None and x["score"] < ev["score"]],
                        key=variants.sort_key)
        if len(better) >= 2:                     # two improving edits together
            top = better[:4]
            pairs = [("%s + %s" % (a["label"], b["label"]), "pair", a["hunks"] + b["hunks"], None)
                     for i, a in enumerate(top) for b in top[i + 1:]]
            prs = [x for x in variants.materialise(pairs, text, len(rows) + 1) if "text" in x]
            variants.run_batch(prs, tgt.meta, tgt.syms, tgt.retail, tgt.inc, args.jobs, False)
            rows += prs
            better = sorted([x for x in rows if x.get("score") is not None and x["score"] < ev["score"]],
                            key=variants.sort_key)
        for x in rows:
            if "text" in x:
                seen.add(hashlib.sha1(x["text"].encode()).digest())
        noedit = sum(1 for x in rows if x.get("status") == "NOEDIT")
        errors = sum(1 for x in rows if x.get("status") == "ERROR")
        built = sum(1 for x in rows if x.get("score") is not None)
        status, note = "no better", "%d edit(s): %d not applicable, %d cc1 error(s), %d built" % (
            len(edits), noedit, errors, built)
        if not edits:
            note = "no edit blocks in the answer: " + " ".join(r["text"].split())[:100]
        prev = ev["score"]
        for x in better:
            if x["score"] == 0:
                ok, defsyms, summ = oracle_ok(tgt, x["text"], work)
                if ok:
                    log.row(job="tweak", unit=tgt.unit, vaddr=tgt.vs, size=tgt.size, model=model, round=k,
                            prompt_tokens=r["prompt_tokens"], completion_tokens=r["completion_tokens"],
                            llm_s="%.1f" % r["seconds"], eval_s="%.1f" % (time.time() - t1),
                            proposals=len(edits), built=built, before=prev, after=0, status="MATCH",
                            note=x["label"])
                    ev = tgt.evaluate(x["text"])
                    return result(tgt, "tweak", model, x["text"], before, ev, calls, t0, status="MATCH",
                                  defsyms=defsyms, note=x["label"])
        if better:
            text, status = better[0]["text"], "better"
            ev = tgt.evaluate(text)
            note = better[0]["label"] + "; " + note
        tried += [x["label"] for x in rows if x.get("score") is None or x["score"] >= prev]
        log.row(job="tweak", unit=tgt.unit, vaddr=tgt.vs, size=tgt.size, model=model, round=k,
                prompt_tokens=r["prompt_tokens"], completion_tokens=r["completion_tokens"],
                llm_s="%.1f" % r["seconds"], eval_s="%.1f" % (time.time() - t1), proposals=len(edits),
                built=built, before=prev, after=ev["score"], status=status, note=note)
    return result(tgt, "tweak", model, text, before, ev, calls, t0,
                  status="better" if ev["score"] < before else "no match")


# ---------------------------------------------------------------------------
# commit (as permute.commit)
# ---------------------------------------------------------------------------
def commit(tgt, res, args):
    unit, vs = tgt.unit, tgt.vs
    cpath = drive.c_path(drive.RE_MATCH, unit, vs)
    if os.path.exists(cpath) or ledger.state(unit, vs) == "match":
        return "skipped: already matched"
    header = ("/* %s %s  size %d  local LLM (tools/loop/golem.py %s, %s), %s */\n"
              % (unit, vs, tgt.size, res["job"], res["model"], datetime.date.today().isoformat()))
    body = res["text"]
    if body.startswith(sweep.NEIGHBOUR_HDR):        # the new header says where it came from
        body = body.partition("\n")[2]
    os.makedirs(os.path.dirname(cpath), exist_ok=True)
    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(header + body)
    out = permute.oracle({"unit": unit, "vaddr": vs}, cpath, tgt.meta)
    clean, defsyms = out[0], out[1]
    if not clean:
        os.unlink(cpath)
        return "re-verify failed (%s)" % out[2]
    extra = []
    if tgt.meta["rodata"]:
        extra += ["--rodata", tgt.meta["rodata"]]
    syms = drive.syms_args(cpath)
    extra += syms
    for d in (drive.drop_table_defsyms(defsyms, drive.table_syms()) if syms else defsyms):
        extra += ["--defsym", d]
    drive.manifest_append(vs, units.resolve(unit).manifest_unit, "%s/%s.c" % (unit, drive.fn_name(vs)),
                          tgt.size, " ".join(extra))
    if not args.no_ledger:
        ledger.record("match", unit, vs, 0, "golem (%s, %s)" % (res["model"], res["job"]))
    return "MATCH"


# ---------------------------------------------------------------------------
# candidates for run
# ---------------------------------------------------------------------------
TRY_ERR = re.compile(r"^(\S+) ERROR")


def read_tsv(path):
    with open(path, encoding="utf-8") as fh:
        header = fh.readline().rstrip("\r\n").split("\t")
        return [dict(zip(header, ln.rstrip("\r\n").split("\t"))) for ln in fh if ln.strip()]


def report_jobs(path, max_words):
    """sweep_report.tsv / permute_report.tsv -> [{unit, vaddr, cfile, job, words}]."""
    d = os.path.dirname(os.path.abspath(path))
    out = []
    for r in read_tsv(path):
        if r.get("status") == "MATCH":
            continue
        tag = "%s_%s" % (r["unit"], r["vaddr"][2:].upper())
        if "tries" in r:                             # sweep report
            tries = [t.strip() for t in r["tries"].split(" | ") if t.strip()]
            mis = [(int(m.group(3)), m.group(1)) for m in (permute.TRY.match(t) for t in tries) if m]
            if mis:
                w, variant = min(mis)
                if w <= max_words:
                    out.append({"unit": r["unit"], "vaddr": r["vaddr"], "job": "tweak", "words": w,
                                "cfile": os.path.join(d, "%s_%s.c" % (tag, variant))})
            elif tries and all(TRY_ERR.match(t) for t in tries):
                variant = TRY_ERR.match(tries[0]).group(1)
                out.append({"unit": r["unit"], "vaddr": r["vaddr"], "job": "fix", "words": None,
                            "cfile": os.path.join(d, "%s_%s.c" % (tag, variant))})
        else:                                        # permute report
            sc = [int(r[k]) for k in ("base", "lever") if r.get(k) not in (None, "", "None")]
            src = next((os.path.join(d, tag, f) for f in ("lever.c", "draft.c")
                        if os.path.exists(os.path.join(d, tag, f))), None)
            if sc and src and min(sc) <= max_words:
                out.append({"unit": r["unit"], "vaddr": r["vaddr"], "job": "tweak", "words": min(sc),
                            "cfile": src})
    return out


def cmd_run(args):
    cands = {}
    for p in args.report or []:
        for c in report_jobs(p, args.max_words):
            k = (c["unit"], "0x%08X" % int(c["vaddr"], 16))
            if k not in cands or (c["words"] or 999) < (cands[k]["words"] or 999):
                cands[k] = c
    for spec in args.cfile or []:
        c = permute.cfile_candidate(spec)
        cands[(c["unit"], c["vaddr"])] = {"unit": c["unit"], "vaddr": c["vaddr"], "job": "auto",
                                          "words": None, "cfile": c["cfile"], "own": True}
    rows = drive.load_rows()
    todo = []
    for k, c in sorted(cands.items(), key=lambda kv: ((kv[1]["words"] or 0), kv[0])):
        row = rows.get(k)
        size = int(row["size"]) if row else 0
        if args.max_size and size > args.max_size:
            continue
        if not c.get("own") and (ledger.state(*k) == "match" or os.path.exists(drive.c_path(drive.RE_MATCH, *k))):
            continue
        if not os.path.exists(c["cfile"]):
            continue
        if args.job != "auto" and c["job"] not in (args.job, "auto"):
            continue
        todo.append(c)
    if args.limit:
        todo = todo[:args.limit]
    log = Log(args.log)
    work = tempfile.mkdtemp(prefix="golem_run_")
    n_match = 0
    t0 = time.time()
    try:
        for i, c in enumerate(todo, 1):
            res = run_one(c, args, args.model, log, work)
            st = res["status"]
            if st == "MATCH" and not args.no_commit:
                st = commit(res["tgt"], res, args)
            n_match += st == "MATCH"
            print("[%3d/%d] %-7s %s %5s B  %-5s %-9s before %-6s after %-6s %3d call(s) %5.0f s  %s"
                  % (i, len(todo), c["unit"], res["vaddr"], res["size"], res["job"], st, res["before"],
                     res["after"], res["calls"], res["wall_s"], res["note"][:60]))
            sys.stdout.flush()
    finally:
        shutil.rmtree(work, ignore_errors=True)
    print("golem: %d/%d matched with %s in %.0f s; log %s" % (n_match, len(todo), args.model, time.time() - t0,
                                                              args.log))
    return 0


def run_one(c, args, model, log, work):
    vaddr = int(c["vaddr"], 16)
    tgt = Target(c["unit"], vaddr, c["cfile"])
    job = c["job"]
    if job == "auto":
        job = "tweak" if tgt.evaluate(tgt.text)["status"] == "built" else "fix"
    res = do_fix(tgt, args, model, log, work) if job == "fix" else do_tweak(tgt, args, model, log, work)
    if (job == "fix" and getattr(args, "then_tweak", 0) and res["status"] == "compiled"
            and res["after"] is not None and res["after"] <= args.max_words):
        t2 = Target(c["unit"], vaddr, c["cfile"])
        t2.text = res["text"]
        tw = do_tweak(t2, args, model, log, work, rounds=args.then_tweak)
        res.update(status=tw["status"], after=tw["after"], text=tw["text"], defsyms=tw["defsyms"],
                   calls=res["calls"] + tw["calls"], wall_s=res["wall_s"] + tw["wall_s"],
                   note="fixed, then tweak: " + tw["note"])
    res["tgt"] = tgt
    return res


# ---------------------------------------------------------------------------
# bench
# ---------------------------------------------------------------------------
BENCH_COLS = ["model", "tier", "job", "unit", "vaddr", "size", "base", "status", "before", "after", "built",
              "calls", "prompt_tokens", "completion_tokens", "llm_s", "wall_s", "perm", "note"]


def cmd_bench(args):
    rows = read_tsv(args.bench)
    out_dir = os.path.abspath(args.out)
    os.makedirs(out_dir, exist_ok=True)
    res_path = os.path.join(out_dir, "bench_results.tsv")
    done = set()
    if os.path.exists(res_path):
        done = {(r["model"], r["unit"], r["vaddr"], r["job"]) for r in read_tsv(res_path)}
    else:
        with open(res_path, "w", encoding="utf-8", newline="\n") as fh:
            fh.write("\t".join(BENCH_COLS) + "\n")
    log = Log(os.path.join(out_dir, "golem_log.tsv") if args.log == DEFAULT_LOG else args.log)
    work = tempfile.mkdtemp(prefix="golem_bench_")
    try:
        for model in args.models.split(","):
            for i, b in enumerate(rows, 1):
                if (model, b["unit"], b["vaddr"], b["job"]) in done:
                    continue
                c = {"unit": b["unit"], "vaddr": b["vaddr"], "job": b["job"], "cfile": b["cfile"]}
                try:
                    res = run_one(c, args, model, log, work)
                except Exception as e:      # noqa: BLE001 - one bad row must not stop the bench
                    res = {"status": "crash", "before": "", "after": "", "built": "", "calls": 0,
                           "prompt_tokens": 0, "completion_tokens": 0, "llm_s": 0, "wall_s": 0,
                           "size": b.get("size", ""), "note": ("%s: %s" % (type(e).__name__, e))[:150]}
                if res["status"] == "MATCH":
                    keep = os.path.join(out_dir, "match", model.replace("/", "_"), b["unit"],
                                        "func_%s.c" % b["vaddr"][2:].upper())
                    os.makedirs(os.path.dirname(keep), exist_ok=True)
                    with open(keep, "w", encoding="utf-8", newline="\n") as fh:
                        fh.write(res["text"])
                line = {"model": model, "tier": b["tier"], "job": b["job"], "unit": b["unit"],
                        "vaddr": b["vaddr"], "size": res["size"], "base": b.get("base", ""),
                        "status": res["status"], "before": res["before"], "after": res["after"],
                        "built": int(bool(res["built"])) if res["built"] != "" else "",
                        "calls": res["calls"], "prompt_tokens": res["prompt_tokens"],
                        "completion_tokens": res["completion_tokens"], "llm_s": "%.0f" % res["llm_s"],
                        "wall_s": "%.0f" % res["wall_s"], "perm": b.get("perm", ""), "note": res["note"]}
                with open(res_path, "a", encoding="utf-8", newline="\n") as fh:
                    fh.write("\t".join(" ".join(str(line[k]).split()) for k in BENCH_COLS) + "\n")
                print("%-26s [%2d/%d] %-2s %-5s %-7s %s %-9s %s -> %s  %d call(s) %4.0f s  %s"
                      % (model[:26], i, len(rows), b["tier"], b["job"], b["unit"], b["vaddr"], res["status"],
                         res["before"], res["after"], res["calls"], float(res["wall_s"]), res["note"][:50]))
                sys.stdout.flush()
    finally:
        shutil.rmtree(work, ignore_errors=True)
    print_summary(read_tsv(res_path))
    return 0


def print_summary(rows):
    print("\n| model | tier | job | rows | matched | improved | compiled | mean words before -> after | "
          "calls | wall s/row |")
    print("|---|---|---|---:|---:|---:|---:|---|---:|---:|")
    keys = sorted({(r["model"], r["tier"], r["job"]) for r in rows})
    for k in keys:
        rs = [r for r in rows if (r["model"], r["tier"], r["job"]) == k]
        ran = [r for r in rs if r["status"] not in ("skipped", "crash")]
        m = sum(r["status"] == "MATCH" for r in rs)
        num = [(int(r["before"]), int(r["after"])) for r in ran
               if r["before"].lstrip("-").isdigit() and r["after"].lstrip("-").isdigit()]
        imp = sum(1 for a, b in num if b < a) if k[2] == "tweak" else 0
        comp = sum(r["built"] == "1" for r in ran) if k[2] == "fix" else 0
        mean = ("%.1f -> %.1f" % (sum(a for a, _ in num) / len(num), sum(b for _, b in num) / len(num))
                if num else "-")
        calls = sum(int(r["calls"] or 0) for r in rs)
        wall = sum(float(r["wall_s"] or 0) for r in rs) / max(1, len(rs))
        print("| %s | %s | %s | %d | %d | %s | %s | %s | %d | %.0f |"
              % (k[0], k[1], k[2], len(rs), m, imp if k[2] == "tweak" else "-",
                 "%d/%d" % (comp, len(ran)) if k[2] == "fix" else "-", mean, calls, wall))


# ---------------------------------------------------------------------------
def add_common(ap):
    ap.add_argument("--model", default=DEFAULT_MODEL, help="model id (default %(default)s)")
    ap.add_argument("--url", default=DEFAULT_URL, help="OpenAI-compatible base URL (default %(default)s)")
    ap.add_argument("--attempts", type=int, default=3, help="fix: answers per draft (default 3)")
    ap.add_argument("--rounds", type=int, default=3, help="tweak: rounds (default 3)")
    ap.add_argument("-n", type=int, default=6, help="tweak: edit blocks asked for per round (default 6)")
    ap.add_argument("--then-tweak", type=int, default=0, metavar="R",
                    help="fix: R tweak rounds after a fix that compiles within --max-words")
    ap.add_argument("--max-words", type=int, default=12, help="tweak only drafts this close (default 12)")
    ap.add_argument("--max-prompt", type=int, default=MAX_PROMPT, help="prompt characters (default %(default)s)")
    ap.add_argument("--timeout", type=int, default=600, help="seconds per model call (default 600)")
    ap.add_argument("-j", "--jobs", type=int, default=8, help="parallel compiles per batch (default 8)")
    ap.add_argument("--log", default=DEFAULT_LOG, help="attempt log TSV (default %(default)s; '' = none)")
    ap.add_argument("--dump", metavar="DIR", help="write every prompt and answer to DIR")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("fix", "tweak"):
        p = sub.add_parser(name, help="one job on one C file (nothing written unless --out)")
        p.add_argument("unit")
        p.add_argument("vaddr", type=match_fn.parse_int)
        p.add_argument("cfile")
        p.add_argument("--out", help="write the best text here")
        add_common(p)
    p = sub.add_parser("run", help="jobs from reports / C files; matches go through manifest and ledger")
    p.add_argument("--report", action="append", help="sweep_report.tsv or permute_report.tsv (repeatable)")
    p.add_argument("--cfile", action="append", metavar="[UNIT:]FILE", help="a C file (job auto)")
    p.add_argument("--job", default="auto", choices=["auto", "fix", "tweak"])
    p.add_argument("--max-size", type=int, default=800, help="bytes (default 800; 0 = any)")
    p.add_argument("--limit", type=int)
    p.add_argument("--no-commit", action="store_true", help="report matches, write nothing")
    p.add_argument("--no-ledger", action="store_true")
    add_common(p)
    p = sub.add_parser("bench", help="every model on every row of a bench TSV, same budget")
    p.add_argument("bench")
    p.add_argument("--models", default=DEFAULT_MODEL, help="comma-separated model ids")
    p.add_argument("--out", required=True, help="results directory (rerun resumes)")
    add_common(p)
    args = ap.parse_args(argv)
    if args.log == "":
        args.log = None

    if args.cmd == "run":
        if not (args.report or args.cfile):
            ap.error("give --report and/or --cfile")
        return cmd_run(args)
    if args.cmd == "bench":
        return cmd_bench(args)
    tgt = Target(units.norm(args.unit), args.vaddr, args.cfile)
    work = tempfile.mkdtemp(prefix="golem_")
    try:
        log = Log(args.log)
        res = (do_fix if args.cmd == "fix" else do_tweak)(tgt, args, args.model, log, work)
    finally:
        shutil.rmtree(work, ignore_errors=True)
    print("%s %s %s: %s, before %s, after %s, %d call(s), %d+%d tokens, %.0f s LLM, %.0f s wall%s"
          % (args.cmd, tgt.unit, tgt.vs, res["status"], res["before"], res["after"], res["calls"],
             res["prompt_tokens"], res["completion_tokens"], res["llm_s"], res["wall_s"],
             ("; " + res["note"]) if res["note"] else ""))
    if args.out and res["text"] != tgt.text:
        with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(res["text"])
        print("wrote %s" % args.out)
    return 0 if res["status"] == "MATCH" else 1


if __name__ == "__main__":
    sys.exit(main())
