#!/usr/bin/env python3
"""m2cfix.py - text fix-ups that make m2c drafts compile under cc1 2.7.2 (tools/loop/sweep.py).

Two kinds:

  * whole-text fix-ups, run on m2c's output before the first compile:
      knr_externs    `/* extern */` prototypes m2c guessed -> unprototyped `T f();`
      declare_locals declare the m2c names it uses but never declares (`sp1A` next to an
                     address-taken `sp18`, `saved_reg_s2`, `unksp3E`), typed from the
                     --stack-structs template when m2c printed one (else M2C_UNK)
      stack_frame    every `spNN` stack variable becomes a field of one local union at
                     its exact offset, so the frame layout is m2c's, not cc1's order
      void_casts     a call whose value is used, to a callee the headers declare void,
                     goes through `((M2C_UNK (*)())f)(...)`; `*(void *)K` -> `*(M2C_UNK *)K`
  * repair(text, errors): cc1's messages (fn.i line, text) mapped back to draft lines,
    and one rule per message kind applied to those lines only (cast the dereferenced
    operand, cast pointers in arithmetic, cast the callee, declare the name).  sweep.py
    runs it between oracle runs until the draft compiles or nothing changes.

Every fix-up keeps m2c's byte arithmetic and only adds casts or declarations; a draft
that compiled before is never touched by repair().
"""
import glob
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
HEADER_GLOBS = [os.path.join(REPO, "include", "ac1", "*.h"),
                os.path.join(REPO, "tools", "match", "include", "*.h")]

EXTERN_PROTO = re.compile(r"^([^\n;()]*?\b\w+)\([^\n;]*\);(\s*/\* extern \*/)$", re.M)
CAST_UNK = "(M2C_UNK *)"
KEYWORDS = {"if", "while", "for", "switch", "return", "sizeof", "do", "else", "case", "goto",
            "M2C_FIELD", "M2C_BITWISE", "M2C_ERROR", "M2C_UNALIGNED32"}


def knr_externs(text):
    """m2c guesses an `/* extern */` callee's parameter list from the registers set
    before the first call (`s32 SquareRoot0(s32, s32);` when a1 happened to be live),
    so a later call with fewer arguments is a cc1 error.  Declare those callees
    unprototyped instead (`s32 SquareRoot0();`): any call compiles, int-sized
    arguments are passed exactly as before."""
    return EXTERN_PROTO.sub(r"\1();\2", text)


# ----------------------------------------------------------------------------
# draft structure
# ----------------------------------------------------------------------------
STACK_STRUCT = re.compile(r"^struct _m2c_stack_\w+ \{\n(.*?)^\};[^\n]*?(?:size = (0x[0-9A-Fa-f]+|\d+))?[^\n]*\n\n?",
                          re.M | re.S)
STACK_FIELD = re.compile(r"/\* (0x[0-9A-Fa-f]+) \*/ (.*?)\b(sp[0-9A-F]+|unksp[0-9A-F]*)\s*(\[[^\]]*\])?;")
DECL = re.compile(r"^    ([A-Za-z_][\w ]*?(?:\s*\*+\s*|\s+))(\w+)\s*(\[[^\]]*\])?;\s*(?:/\*.*\*/)?$")


def stack_template(text):
    """(text without the --stack-structs template, {name: (offset, type, dims)}, frame size)."""
    m = STACK_STRUCT.search(text)
    if not m:
        return text, {}, None
    fields = {}
    for f in STACK_FIELD.finditer(m.group(1)):
        fields[f.group(3)] = (int(f.group(1), 16), f.group(2).strip(), f.group(4) or "")
    size = int(m.group(2), 0) if m.group(2) else None
    return text[:m.start()] + text[m.end():], fields, size


def function_span(text, name):
    """(def line index, '{' line index, '}' line index) of NAME's definition, by lines."""
    lines = text.split("\n")
    for i, ln in enumerate(lines):
        if re.match(r"^\S[^;]*\b%s\(" % re.escape(name), ln) and ln.rstrip().endswith("{"):
            for j in range(i + 1, len(lines)):
                if lines[j].startswith("}"):
                    return i, i, j
    return None


def declared_names(lines, body_start):
    """{name: type-with-dims} of the declaration block right after the opening brace
    (m2c ends it with a blank line), and the index of the line after it."""
    names, i = {}, body_start + 1
    while i < len(lines) and re.sub(r"/\*.*?\*/", "", lines[i]).rstrip().endswith(";"):
        m = DECL.match(lines[i])
        if m:
            names[m.group(2)] = m.group(1).strip() + ((" " + m.group(3)) if m.group(3) else "")
        else:
            fp = re.search(r"\(\*\s*(\w+)\)", lines[i])
            if not fp:
                break
            names[fp.group(1)] = "fnptr *"
        i += 1
    return names, i


def params(def_line):
    m = re.search(r"\(([^()]*)\)\s*\{", def_line)
    out = {}
    if m:
        for p in m.group(1).split(","):
            pm = re.match(r"\s*(.*?)\b(\w+)\s*$", p)
            if pm and pm.group(2) != "void":
                out[pm.group(2)] = pm.group(1).strip()
    return out


def file_globals(text):
    """Names declared at file scope (externs, prototypes, m2c's `extern` data)."""
    names = set()
    for ln in text.split("\n"):
        if ln.startswith((" ", "\t", "#", "}")) or not ln.strip():
            continue
        for m in re.finditer(r"\b(\w+)\s*(?:\(|\[|;|=)", ln):
            names.add(m.group(1))
    return names


M2C_NAME = re.compile(r"(?<![\w.>])(sp[0-9A-F]*|unksp[0-9A-F]*|saved_reg_\w+|(?:temp|var|phi)_\w+)\b(?!\s*\()")


# ----------------------------------------------------------------------------
# whole-text fix-ups
# ----------------------------------------------------------------------------
def _decl(ctype, name, dims=""):
    sep = "" if ctype.endswith("*") else " "
    return "    %s%s%s%s;" % (ctype, sep, name, dims)


def declare_locals(text, name, row=None):
    """Declare every m2c-style name the function body uses but m2c never declared."""
    text, fields, frame = stack_template(text)
    span = function_span(text, name)
    if not span:
        return text
    lines = text.split("\n")
    _d, start, end = span
    have, after = declared_names(lines, start)
    have.update(params(lines[start]))
    glob_names = file_globals(text)
    missing = []
    for ln in lines[after:end]:
        for m in M2C_NAME.finditer(_strip_strings(ln)):
            n = m.group(1)
            if n not in have and n not in glob_names and n not in missing:
                missing.append(n)
    if not missing:
        return text
    new = []
    for n in missing:
        if n in fields:
            new.append(_decl(fields[n][1] or "M2C_UNK", n, fields[n][2]))
        elif n == "sp":
            new.append(_decl("s8", "sp", "[0x%X]" % (frame or 8)))
        else:
            new.append(_decl("M2C_UNK", n))
    return "\n".join(lines[:start + 1] + new + lines[start + 1:])


def _strip_strings(s):
    return re.sub(r'"(?:\\.|[^"\\])*"', '""', re.sub(r"/\*.*?\*/", "", s))


TYPE_SIZE = {"s8": 1, "u8": 1, "M2C_UNK8": 1, "char": 1, "s16": 2, "u16": 2, "M2C_UNK16": 2,
             "short": 2, "s64": 8, "u64": 8, "M2C_UNK64": 8, "f64": 8, "double": 8}


def frame_saves(blob):
    """(frame size, lowest offset of a saved register) from the prologue words, or Nones."""
    import struct
    frame, low = None, None
    for i in range(0, min(len(blob), 4 * 24), 4):
        w = struct.unpack_from("<I", blob, i)[0]
        op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
        if op == 9 and rs == 29 and rt == 29 and imm & 0x8000 and frame is None:
            frame = 0x10000 - imm
        elif op == 0x2B and rs == 29 and (16 <= rt <= 23 or rt in (30, 31)):
            low = imm if low is None else min(low, imm)
    return frame, low


def stack_frame(text, name, row=None, blob=None):
    """All `spNN` variables below the register save area -> fields of one local union
    `sp_frame` at (offset - lowest offset), so cc1 lays the frame out as the retail code
    did (one local, placed right above the outgoing-argument area)."""
    text = declare_locals(text, name, row)        # every name declared, template parsed
    span = function_span(text, name)
    if not span:
        return text
    lines = text.split("\n")
    _d, start, end = span
    have, after = declared_names(lines, start)
    frame, low = frame_saves(blob) if blob else (None, None)
    top = low if low is not None else frame
    stack = {}
    for n, ctype in have.items():
        m = re.fullmatch(r"sp([0-9A-F]+)", n)
        if m and (top is None or int(m.group(1), 16) < top):
            stack[n] = (int(m.group(1), 16), ctype)
    if not stack:
        return text
    base = min(off for off, _t in stack.values())
    if top is None:
        top = max(off + TYPE_SIZE.get(t.split("[")[0].strip(), 4) for off, t in stack.values())
    size = max(4, top - base)
    keep = [ln for ln in lines[start + 1:after] if (DECL.match(ln) and DECL.match(ln).group(2)) not in stack]
    union = "    union { s8 b[0x%X]; s32 w; } sp_frame;" % size

    def ref(n):
        off, ctype = stack[n]
        dims = re.search(r"\[.*\]", ctype)
        t = ctype.split("[")[0].strip()
        if dims:
            return "((%s *)(sp_frame.b + 0x%X))" % (t, off - base)
        return "(*(%s *)(sp_frame.b + 0x%X))" % (t, off - base)
    pat = re.compile(r"(?<![\w.>])(%s)\b" % "|".join(sorted(stack, key=len, reverse=True)))
    body = [pat.sub(lambda m: ref(m.group(1)), ln) for ln in lines[after:end]]
    return "\n".join(lines[:start + 1] + [union] + keep + body + lines[end:])


_VOID_FNS = None


def void_functions():
    """Names the shared headers declare `void f(...)` (not `void *f`)."""
    global _VOID_FNS
    if _VOID_FNS is None:
        names = set()
        for g in HEADER_GLOBS:
            for p in glob.glob(g):
                with open(p, encoding="utf-8", errors="replace") as fh:
                    for m in re.finditer(r"^\s*(?:extern\s+)?void\s+(\w+)\s*\(", fh.read(), re.M):
                        names.add(m.group(1))
        _VOID_FNS = names
    return _VOID_FNS


def cast_callee(line, fn):
    return re.sub(r"(?<![\w.>)])\b%s\s*\(" % re.escape(fn), "((M2C_UNK (*)())%s)(" % fn, line)


def void_casts(text, name=None, row=None):
    """`x = f(...)` where the headers say `void f(...)`: call f as returning M2C_UNK
    (same `jal`); and `*(void *)K` (m2c's untyped scratchpad access) as M2C_UNK."""
    text = text.replace("*(void *)", "*(M2C_UNK *)")
    voids = void_functions()
    out = []
    for ln in text.split("\n"):
        if ln.startswith((" ", "\t")):
            for fn in voids:
                if re.search(r"\b%s\s*\(" % re.escape(fn), ln) and not re.match(r"^\s*%s\s*\(" % re.escape(fn), ln):
                    ln = cast_callee(ln, fn)
        out.append(ln)
    return "\n".join(out)


# ----------------------------------------------------------------------------
# error-driven repair
# ----------------------------------------------------------------------------
def code_lines(text):
    """Indices of the draft lines that survive into fn.i (cpp -P drops blank lines,
    comments and directives); those are the tail of fn.i, after the headers."""
    t = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    t = re.sub(r"//[^\n]*", "", t)
    lines = t.split("\n")
    last_inc = max([i for i, ln in enumerate(lines) if ln.lstrip().startswith("#")] or [-1])
    return [i for i in range(last_inc + 1, len(lines)) if lines[i].strip()]


def map_errors(text, fn_i, errors):
    """[(fn.i line, msg)] -> [(draft line index, msg)] (lines before the draft dropped)."""
    with open(fn_i, encoding="utf-8", errors="replace") as fh:
        n_i = sum(1 for ln in fh.read().split("\n") if ln.strip())
    idx = code_lines(text)
    first = n_i - len(idx) + 1                       # fn.i line of the first draft code line
    out = []
    for ln, msg in errors:
        k = ln - first
        if 0 <= k < len(idx):
            out.append((idx[k], msg))
    return out


CC1_MSG = re.compile(r"fn\.i:(\d+): (.*)")


def cc1_errors(err_text):
    out = []
    for m in CC1_MSG.finditer(err_text or ""):
        msg = m.group(1), m.group(2).strip()
        if msg[1].startswith("warning") or msg[1].startswith("(") or "for each function" in msg[1]:
            continue
        out.append((int(msg[0]), msg[1]))
    return out


# what may stand before a unary operator: start, an operator, `return`, or a cast to a
# scalar type (`(u8) *p`); `)` or a name before it means the operator is binary
UNARY_PREFIX = (r"(^\s*|[=(,!~+\-*/%|^<>?:;{]\s*|&&\s*|\breturn\s+"
                r"|\((?:[su](?:8|16|32|64)|M2C_UNK\d*|f32|f64)\s*\**\)\s*)")
UNARY_STAR = re.compile(UNARY_PREFIX + r"\*(?=\s*[\w(*])(?!\s*\(\s*[A-Za-z_][\w ]*(?:\*\s*)+\))")
UNARY_AMP = re.compile(UNARY_PREFIX + r"&(?!&)(\w+)\b(?!\s*(?:->|\.|\[))")


def cast_derefs(line, ptr_ok):
    """Every unary `*` on the line whose operand is not a cast `(T *)` or a typed pointer
    variable becomes `*(M2C_UNK *)`: m2c lost the pointer type (an s32 field, a void *)."""
    def sub(m):
        rest = line[m.end():]
        im = re.match(r"\s*(\w+)\b(?!\s*(?:->|\.|\[|\())", rest)
        if im and im.group(1) in ptr_ok:
            return m.group(0)
        return m.group(1) + "*" + CAST_UNK
    return UNARY_STAR.sub(sub, line)


def cast_pointer_operands(line, ptrs, ptr2=()):
    """Pointers used in integer arithmetic (`p - q` of two types, `K - &D`, `p * 4`,
    `*pp * 4`, `x - g_table`) -> (s32); ptr2 are the pointer-to-pointer locals."""
    line = re.sub(r"\*\(\s*\w[\w ]*?\s*\*\s*\*\s*\)", "*(s32 *)", line)
    line = UNARY_AMP.sub(lambda m: "%s((s32) &%s)" % (m.group(1), m.group(2)), line)
    if ptr2:
        pat = re.compile(UNARY_PREFIX + r"\*\s*(%s)\b(?!\s*(?:->|\.|\[|\())"
                         % "|".join(sorted(ptr2, key=len, reverse=True)))
        line = pat.sub(lambda m: "%s*(s32 *)%s" % (m.group(1), m.group(2)), line)
    line = re.sub(r"(?<![\w.>*&])(?<!\(s32\) )\b((?:D|g)_\w+)\b"
                  r"(?!\s*(?:->|\.|\[|\(|(?:[-+*/%&|^]|<<|>>)?=(?!=)))", r"((s32) \1)", line)
    if ptrs:
        pat = re.compile(r"(?<![\w.>*&])(?<!\(s32\) )(?<!\*\))\b(%s)\b(?!\s*(?:->|\.|\[|\(|(?:[-+*/%%&|^]|<<|>>)?=(?!=)|\+\+|--))"
                         % "|".join(sorted(ptrs, key=len, reverse=True)))
        line = pat.sub(lambda m: "((s32) %s)" % m.group(1), line)
    return line


def repair(text, name, errors, row=None):
    """Apply one rule per cc1 message to the draft lines it names -> new text, or None
    when no rule applies (the caller stops)."""
    span = function_span(text, name)
    if not span:
        return None
    lines = text.split("\n")
    _d, start, end = span
    have, after = declared_names(lines, start)
    have.update(params(lines[start]))
    ptrs = {n for n, t in have.items() if "*" in t or "[" in t}
    ptr2 = {n for n, t in have.items() if t.count("*") >= 2}
    typed_ptrs = {n for n, t in have.items() if "*" in t and not t.startswith("void")}
    _t, fields, _f = stack_template(text)
    changed, declare = set(), []
    for li, msg in errors:
        old = lines[li]
        new = old
        um = re.match(r"`(\w+)' undeclared", msg)
        fm = re.match(r"too (?:few|many) arguments to function `(\w+)'", msg)
        if um:
            n = um.group(1)
            if n not in have and n not in declare:
                declare.append(n)
            continue
        if fm:
            new = cast_callee(old, fm.group(1))
        elif "unary *" in msg or "void value not ignored" in msg or "invalid use of void" in msg:
            for fn in void_functions():
                if re.search(r"\b%s\s*\(" % re.escape(fn), new):
                    new = cast_callee(new, fn)
            if new == old:
                new = cast_derefs(new.replace("*(void *)", "*" + CAST_UNK), typed_ptrs)
        elif msg.startswith("invalid operands to binary") or msg.startswith("array subscript is not"):
            new = cast_pointer_operands(old, ptrs, ptr2)
        elif msg.startswith("called object is not a function"):
            continue
        if new != old:
            lines[li] = new
            changed.add(li)
    if declare:
        add = []
        for n in declare:
            if n in fields:
                add.append(_decl(fields[n][1] or "M2C_UNK", n, fields[n][2]))
            else:
                add.append(_decl("M2C_UNK", n))
        lines = lines[:start + 1] + add + lines[start + 1:]
    if not changed and not declare:
        return None
    return "\n".join(lines)
