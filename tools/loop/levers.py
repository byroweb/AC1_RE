#!/usr/bin/env python3
"""levers.py - source rewrites for near-miss m2c drafts (the permuter tier's first stage).

Each lever takes the draft text and the function name and returns a list of
(label, new_text) variants; permute.py scores every variant with the oracle's
compile and keeps the best few (beam search) before decomp-permuter takes over.
They are the GCC 2.7.2 / m2c divergences that the pilot (tools/loop/README.md,
permute.py) showed decomp-permuter's random passes almost never reach:

  index   `M2C_FIELD((E + 0xHHHH0000), T *, OFF)` -> a named array `D_<addr>`:
          retail `lui at; addu at,at,idx` keeps the symbol first, m2c's constant
          address gives `addu at,idx,at` (match-function skill: indexed global, p12)
  ptype   pointer locals / externs whose pointee type is wrong: m2c's `M2C_UNK *`
          (s32 here) arithmetic is meant in bytes, so `p += 0xA` came out x4; also
          s8 vs u8 (`li v0,255` vs `li v0,-1`), s16 vs u16 (lh vs lhu)
  etype   `extern M2C_UNK D_x;` scalars used as byte bases (`&D_x + 8`) -> u8/s8/...
  const   integer literals in their other-signedness spelling (-1 <-> 0xFF/0xFFFF,
          -16 <-> 0xF0), which flips `li` immediates and andi/sign-extension pairs
  swap    two adjacent statements of the function body (register colouring follows
          first use: p01/p02; statement order is source order: perturbed test)
  commute operands of a top-level `+ | & ^ *` in one statement (p20)

Levers only touch the function's own definition and the declarations above it.
"""
import re

PTR_TYPES = ["u8", "s8", "u16", "s16", "u32", "s32", "void"]
SCALAR_TYPES = ["u8", "s8", "u16", "s16", "u32", "s32"]
SIZEOF = {"s8": 1, "u8": 1, "M2C_UNK8": 1, "s16": 2, "u16": 2, "M2C_UNK16": 2,
          "s32": 4, "u32": 4, "M2C_UNK": 4, "M2C_UNK32": 4}
KNOWN = set(SIZEOF) | {"void"}


def split_args(s):
    """Top-level comma split of a macro argument list (no surrounding parens)."""
    out, depth, cur = [], 0, []
    for ch in s:
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == "," and depth == 0:
            out.append("".join(cur).strip())
            cur = []
        else:
            cur.append(ch)
    out.append("".join(cur).strip())
    return out


def close_paren(s, i):
    """Index just past the `)` matching the `(` at s[i]."""
    depth = 0
    for j in range(i, len(s)):
        if s[j] == "(":
            depth += 1
        elif s[j] == ")":
            depth -= 1
            if depth == 0:
                return j + 1
    return -1


def strip_parens(s):
    s = s.strip()
    while s.startswith("(") and close_paren(s, 0) == len(s):
        s = s[1:-1].strip()
    return s


def top_split(s, op):
    """Split s at its last top-level binary `op` (spaced, as m2c prints) -> (a, b) or None."""
    depth, k = 0, None
    pat = " %s " % op
    for j in range(len(s)):
        ch = s[j]
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif depth == 0 and s.startswith(pat, j):
            k = j
    if k is None:
        return None
    return s[:k].strip(), s[k + len(pat):].strip()


def parse_int(s):
    s = s.strip()
    try:
        return int(s, 0)
    except ValueError:
        return None


def fn_span(text, fn):
    """(start of body '{', end) of fn's definition, or None."""
    for m in re.finditer(r"\b%s\s*\(" % re.escape(fn), text):
        j = close_paren(text, m.end() - 1)
        k = j
        while 0 <= k < len(text) and text[k].isspace():
            k += 1
        if j < 0 or k >= len(text) or text[k] != "{":
            continue
        depth = 0
        for e in range(k, len(text)):
            if text[e] == "{":
                depth += 1
            elif text[e] == "}":
                depth -= 1
                if depth == 0:
                    return k, e + 1
    return None


def declared(text, name):
    return re.search(r"\b%s\b\s*(\[|;|=)" % re.escape(name), text) is not None


def add_decl(text, fn, decl):
    """Put `decl` on its own line above fn's definition line."""
    sp = fn_span(text, fn)
    if not sp:
        return None
    line = text.rfind("\n", 0, text.rfind(fn, 0, sp[0])) + 1
    return text[:line] + decl + "\n" + text[line:]


# ---------------------------------------------------------------------------
def lever_index(text, fn):
    sp = fn_span(text, fn)
    if not sp:
        return []
    out = []
    body = text[sp[0]:sp[1]]
    for m in re.finditer(r"M2C_FIELD\(", body):
        a = m.start()
        e = close_paren(body, m.end() - 1)
        if e < 0:
            continue
        args = split_args(body[m.end():e - 1])
        if len(args) != 3:
            continue
        base, tptr, off = strip_parens(args[0]), args[1].strip(), parse_int(args[2])
        if off is None or not tptr.endswith("*"):
            continue
        parts = top_split(base, "+")
        if not parts:
            continue
        idx, hi = parts
        hv = parse_int(hi)
        if hv is None or hv & 0xFFFF or not (0x80000000 <= hv < 0x80800000):
            hv2 = parse_int(idx)                      # (0x801F0000 + E) spelling
            if hv2 is None or hv2 & 0xFFFF or not (0x80000000 <= hv2 < 0x80800000):
                continue
            idx, hv = hi, hv2
        addr = (hv + off) & 0xFFFFFFFF
        name = "D_%08X" % addr
        t = tptr[:-1].strip()
        size = 4 if t.endswith("*") else SIZEOF.get(t)
        forms = []
        if not declared(text, name):
            # byte-offset form on a u8 array: always the same address arithmetic
            forms.append(("u8[]", "extern u8 %s[];" % name, "(*(%s)(%s + (%s)))" % (tptr, name, idx)))
            em = None
            for pat in (r"^\((.*)\) \* (\w+)$", r"^(.*) \* (\w+)$", r"^\((.*)\) << (\w+)$", r"^(.*) << (\w+)$"):
                em = re.match(pat, strip_parens(idx))
                if em:
                    break
            if em and size:
                k = parse_int(em.group(2))
                if k is not None and "<<" in em.group(0):
                    k = 1 << k
                if k == size:
                    forms.append(("%s[]" % t, "extern %s %s[];" % (t, name), "%s[%s]" % (name, em.group(1))))
            if size == 1:
                forms.append(("%s[]" % t, "extern %s %s[];" % (t, name), "%s[%s]" % (name, idx)))
        for label, decl, expr in forms:
            nb = body[:a] + expr + body[e:]
            nt = text[:sp[0]] + nb + text[sp[1]:]
            nt = add_decl(nt, fn, decl)
            if nt:
                out.append(("index %s %s" % (name, label), nt))
    return out


DECL_PTR = re.compile(r"^([ \t]*)((?:static |extern )?)(M2C_UNK|M2C_UNK8|M2C_UNK16|M2C_UNK32|[su](?:8|16|32)|void) \*(\w+);",
                      re.M)
DECL_SCALAR = re.compile(r"^extern (M2C_UNK|M2C_UNK8|M2C_UNK16|M2C_UNK32|[su](?:8|16|32)) (D_\w+);", re.M)


def lever_ptype(text, fn):
    out = []
    for m in DECL_PTR.finditer(text):
        cur = m.group(3)
        for t in PTR_TYPES:
            if t == cur:
                continue
            nt = text[:m.start(3)] + t + text[m.end(3):]
            out.append(("ptype %s %s->%s" % (m.group(4), cur, t), nt))
    return out


def lever_etype(text, fn):
    out = []
    for m in DECL_SCALAR.finditer(text):
        cur = m.group(1)
        for t in SCALAR_TYPES:
            if t != cur:
                out.append(("etype %s %s->%s" % (m.group(2), cur, t), text[:m.start(1)] + t + text[m.end(1):]))
    return out


LIT = re.compile(r"(?<![\w.])(-?)(0x[0-9A-Fa-f]+|\d+)(?![\w.])")


def lever_const(text, fn):
    sp = fn_span(text, fn)
    if not sp:
        return []
    out = []
    body = text[sp[0]:sp[1]]
    for m in LIT.finditer(body):
        v = int(m.group(2), 0) * (-1 if m.group(1) else 1)
        # a '-' right after an operand is subtraction, not a negative literal
        if m.group(1) and re.search(r"[\w)\]]\s*$", body[:m.start()]):
            continue
        alts = []
        if -128 <= v < 0:
            alts += [v + 0x100, v + 0x10000]
        elif -32768 <= v < -128:
            alts += [v + 0x10000]
        elif 0x80 <= v <= 0xFF:
            alts += [v - 0x100]
        elif 0x8000 <= v <= 0xFFFF:
            alts += [v - 0x10000]
        for alt in alts:
            s = ("0x%X" % alt) if alt >= 0 else str(alt)
            nb = body[:m.start()] + s + body[m.end():]
            out.append(("const %s->%s" % (m.group(0), s), text[:sp[0]] + nb + text[sp[1]:]))
    return out


STMT = re.compile(r"^([ \t]+)([^\n{};]*;)[ \t]*$", re.M)


def lever_swap(text, fn):
    sp = fn_span(text, fn)
    if not sp:
        return []
    body = text[sp[0]:sp[1]]
    stmts = [m for m in STMT.finditer(body)
             if not re.match(r"\s*(return|break|continue|goto|case|default)\b", m.group(2))
             and not re.match(r"\s*(?:[su](?:8|16|32)|M2C_\w+|void|\w+ \*)\s+\**\w+;", m.group(2))]
    out = []
    for a, b in zip(stmts, stmts[1:]):
        between = body[a.end():b.start()]
        if between.strip() or a.group(1) != b.group(1):
            continue
        nb = body[:a.start()] + b.group(0) + between + a.group(0) + body[b.end():]
        out.append(("swap %s / %s" % (a.group(2).strip()[:24], b.group(2).strip()[:24]),
                    text[:sp[0]] + nb + text[sp[1]:]))
    return out


def lever_commute(text, fn):
    sp = fn_span(text, fn)
    if not sp:
        return []
    body = text[sp[0]:sp[1]]
    out = []
    for m in STMT.finditer(body):
        st = m.group(2)
        eq = re.match(r"^(.*?[^=!<>+\-*/&|^]=(?!=))\s*(.*);$", st)
        lhs, rhs = (eq.group(1), eq.group(2)) if eq else ("", st[:-1])
        for op in ("+", "|", "&", "^", "*"):
            parts = top_split(strip_parens(rhs) if rhs.startswith("(") and close_paren(rhs, 0) == len(rhs)
                              else rhs, op)
            if not parts or not parts[0] or not parts[1]:
                continue
            new = "%s %s %s" % (parts[1], op, parts[0])
            nst = (lhs + " " + new + ";") if lhs else (new + ";")
            nb = body[:m.start(2)] + nst + body[m.end(2):]
            out.append(("commute %s" % st[:30], text[:sp[0]] + nb + text[sp[1]:]))
    return out


PTR_DECL = re.compile(r"(?:^|[(,{;])\s*((?:const\s+)?(?:unsigned\s+|signed\s+)?[A-Za-z_]\w*)\s*\*\s*(\w+)\s*(?=[;,)=])",
                      re.M)
EXT_DECL = re.compile(r"^extern ((?:unsigned\s+)?[A-Za-z_]\w*) (D_\w+);", re.M)
NUM = r"(0x[0-9A-Fa-f]+|\d+)"


def operand_before(s, j):
    """Start of the operand ending at s[j-1]: an identifier/number or a balanced (...) group."""
    k = j
    while k > 0 and s[k - 1] == " ":
        k -= 1
    if k > 0 and s[k - 1] == ")":
        depth = 0
        for i in range(k - 1, -1, -1):
            depth += {")": 1, "(": -1}.get(s[i], 0)
            if depth == 0:
                return i
        return None
    i = k
    while i > 0 and (s[i - 1].isalnum() or s[i - 1] == "_"):
        i -= 1
    return i if i < k else None


def operand_after(s, j):
    """End of the operand starting at s[j]: an identifier/number or a balanced (...) group."""
    if j < len(s) and s[j] == "(":
        e = close_paren(s, j)
        return e if e > 0 else None
    i = j
    while i < len(s) and (s[i].isalnum() or s[i] == "_"):
        i += 1
    return i if i > j else None


def lever_bytes(text, fn):
    """m2c's pointer arithmetic is in bytes: `p += 8` on an `s16 *` was meant as 8 bytes
    (retail `addiu s1,s1,8`, ours 16).  Rewrite one site as byte arithmetic keeping the
    pointee type, or as the scaled element count when it divides."""
    sp = fn_span(text, fn)
    if not sp:
        return []
    sig = text.rfind("\n", 0, text.rfind(fn, 0, sp[0])) + 1
    head = text[sig:sp[1]]                         # signature + body: params and locals
    ptrs = {}
    for m in PTR_DECL.finditer(head):
        ptrs.setdefault(m.group(2), m.group(1))
    exts = {m.group(2): m.group(1) for m in EXT_DECL.finditer(text)}
    body = text[sp[0]:sp[1]]
    out = []

    def emit(label, a, b, new):
        out.append(("bytes " + label, text[:sp[0]] + body[:a] + new + body[b:] + text[sp[1]:]))

    for name, t in ptrs.items():
        k = SIZEOF.get(t)
        if t in ("u8", "s8", "M2C_UNK8", "void", "char"):
            continue
        for m in re.finditer(r"\b%s (\+|-)= %s;" % (re.escape(name), NUM), body):
            n = int(m.group(2), 0)
            emit("%s %s= %s" % (name, m.group(1), m.group(2)), m.start(), m.end(),
                 "%s = (%s *)((u8 *)%s %s %s);" % (name, t, name, m.group(1), m.group(2)))
            if k and n % k == 0:
                emit("%s %s= %d" % (name, m.group(1), n // k), m.start(), m.end(),
                     "%s %s= %d;" % (name, m.group(1), n // k))
        for m in re.finditer(r"(?<![\w*.>])%s \+ %s\b(?! \*)" % (re.escape(name), NUM), body):
            n = int(m.group(1), 0)
            emit("%s + %s" % (name, m.group(1)), m.start(), m.end(),
                 "((%s *)((u8 *)%s + %s))" % (t, name, m.group(1)))
            if k and n % k == 0:
                emit("%s + %d" % (name, n // k), m.start(), m.end(), "(%s + %d)" % (name, n // k))
    for name, t in exts.items():
        if t in ("u8", "s8", "M2C_UNK8", "char"):
            continue
        for m in re.finditer(r"&%s\b" % re.escape(name), body):
            a, b = m.start(), m.end()
            if body[b:b + 3] == " + ":                 # &D + E
                e = operand_after(body, b + 3)
                if e:
                    emit("&%s + E" % name, a, e, "((%s *)((u8 *)&%s + %s))" % (t, name, body[b + 3:e]))
            if body[max(0, a - 3):a] == " + ":         # E + &D
                s = operand_before(body, a - 3)
                if s is not None:
                    emit("E + &%s" % name, s, b, "((%s *)((u8 *)&%s + %s))" % (t, name, body[s:a - 3]))
    return out


LEVERS = {"index": lever_index, "bytes": lever_bytes, "ptype": lever_ptype, "etype": lever_etype,
          "const": lever_const, "swap": lever_swap, "commute": lever_commute}
DEFAULT_LEVERS = "index,bytes,ptype,etype,const,swap,commute"


def variants(text, fn, names=DEFAULT_LEVERS):
    out = []
    for n in names.split(","):
        n = n.strip()
        if n:
            out += LEVERS[n](text, fn)
    return out
