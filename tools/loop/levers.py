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
  args    a call's argument passed through in the caller's own register, which m2c
          drops (retail `li a1,18` with a0 = the caller's arg0, ours `li a0,18`)

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


CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
NOT_CALLEES = {"if", "while", "for", "switch", "return", "sizeof"}


def params(text, fn, sp):
    """[(type, name)] of fn's own parameters (`void` / unnamed -> [])."""
    head = text[:sp[0]]
    m = None
    for m in re.finditer(r"\b%s\s*\(" % re.escape(fn), head):
        pass
    if not m:
        return []
    e = close_paren(head, m.end() - 1)
    out = []
    for a in split_args(head[m.end():e - 1]):
        nm = re.search(r"(\w+)\s*$", a)
        if a.strip() in ("", "void") or not nm or nm.group(1) == a.strip():
            continue
        out.append((a[:nm.start()].strip(), nm.group(1)))
    return out


def lever_args(text, fn):
    """m2c guesses a callee's arity from the argument registers set before the `jal`, so
    an argument passed straight through in the caller's own register (retail `li a1,18`
    with a0 still the caller's arg0, ours `li a0,18`) is dropped.  At one call site, put
    the caller's leading parameter(s) in front (`f(x)` -> `f(arg0, x)`), or its next
    parameter(s) after (`f(arg0)` -> `f(arg0, arg1)`), with the callee's prototype in
    this file widened to match, or made unprototyped; a callee declared elsewhere (the
    shared headers) is called through an unprototyped cast."""
    sp = fn_span(text, fn)
    if not sp:
        return []
    ps = params(text, fn, sp)
    if not ps:
        return []
    body = text[sp[0]:sp[1]]
    head = text[:sp[0]]
    out = []
    for m in CALL.finditer(body):
        callee = m.group(1)
        if callee in NOT_CALLEES or callee == fn or callee.startswith("M2C_"):
            continue
        e = close_paren(body, m.end() - 1)
        if e < 0:
            continue
        inner = body[m.end():e - 1]
        args = [a for a in split_args(inner) if a] if inner.strip() else []
        proto = None
        for pm in re.finditer(r"^([^\n;{}()#]*?)\b%s\(([^()\n]*)\);" % re.escape(callee), head, re.M):
            proto = pm
        if proto is None and not re.match(r"func_[0-9A-Fa-f]{8}$", callee):
            continue                                   # macros, PsyQ calls the headers type
        ptypes = [] if proto is None else [t for t in split_args(proto.group(2)) if t and t != "void"]
        n = len(args)
        tries = []
        for k in (1, 2):
            if k <= len(ps) and n + k <= 6:
                tries.append(("+%s" % ",".join(nm for _t, nm in ps[:k]), [nm for _t, nm in ps[:k]] + args,
                              [t or "s32" for t, _nm in ps[:k]] + ptypes))
            if n + k <= len(ps):
                tries.append(("%s+" % ",".join(nm for _t, nm in ps[n:n + k]), args + [nm for _t, nm in ps[n:n + k]],
                              ptypes + [t or "s32" for t, _nm in ps[n:n + k]]))
        for tag, new_args, new_types in tries:
            if proto is None:
                call = "((M2C_UNK (*)())%s)(%s)" % (callee, ", ".join(new_args))
                forms = [("cast", None)]
            else:
                ret = proto.group(1)
                forms = [("proto", "%s%s(%s);" % (ret, callee, ", ".join(new_types))),
                         ("noproto", "%s%s();" % (ret, callee))]
                call = "%s(%s)" % (callee, ", ".join(new_args))
            nb = body[:m.start()] + call + body[e:]
            for how, decl in forms:
                nh = head if decl is None else head[:proto.start()] + decl + head[proto.end():]
                out.append(("args %s %s %s" % (callee, tag, how), nh + nb + text[sp[1]:]))
    return out


# ---------------------------------------------------------------------------
# diff-driven levers: they read the retail / ours instructions (diff_facts) and put
# retail's numbers and comparisons into the C
# ---------------------------------------------------------------------------
MEM_OPS = {"lw", "lh", "lhu", "lb", "lbu", "sw", "sh", "sb", "lwl", "lwr", "swl", "swr"}
IMM_OPS = {"li", "addiu", "addi", "slti", "sltiu", "andi", "ori", "xori", "lui", "sll", "srl", "sra"}
CMP_OPS = {"slt", "sltu", "slti", "sltiu", "beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez"}
ASM_NUM = r"-?(?:0x[0-9a-f]+|\d+)"


def ins_imm(ins):
    """`addiu v0,v1,16` -> ('addiu', ['v0', 'v1'], 16); `lw v1,776(v0)` -> ('lw', ['v1', 'v0'], 776);
    None for an instruction without an immediate operand."""
    m = re.match(r"^(\w+)\s+(.*)$", ins.strip())
    if not m:
        return None
    mn, ops = m.group(1), [o.strip() for o in m.group(2).split(",")]
    if mn in MEM_OPS:
        mm = re.match(r"^(%s)\((\w+)\)$" % ASM_NUM, ops[-1])
        return (mn, ops[:-1] + [mm.group(2)], int(mm.group(1), 0)) if mm else None
    if mn in IMM_OPS and re.fullmatch(ASM_NUM, ops[-1]):
        return mn, ops[:-1], int(ops[-1], 0)
    return None


def lui_value(rows, i):
    """The constant a `lui` at rows[i] builds with the addiu/ori (or load/store offset)
    on the same register within the next 4 rows."""
    mn, ops, hi = ins_imm(rows[i])
    reg, val = ops[0], hi << 16
    for ins in rows[i + 1:i + 5]:
        p = ins_imm(ins)
        if not p:
            continue
        if p[0] in ("addiu", "ori") and len(p[1]) == 2 and p[1][1] == reg:
            return val + p[2] if p[0] == "addiu" else val | p[2]
        if p[0] in MEM_OPS and p[1][-1] == reg:
            return val + p[2]
    return val


def aligned_pairs(retail, ours):
    """Indices (i, j) of retail / ours instructions that stand for each other: position by
    position when the sizes agree, else the equal-length replace blocks of difflib."""
    import difflib
    if len(retail) == len(ours):
        return [(i, i) for i in range(len(retail))]
    norm = [re.sub(r"0x8[0-9a-f]{7}$", "L", x) for x in retail], [re.sub(r"0x8[0-9a-f]{7}$", "L", x) for x in ours]
    out = []
    for op, i1, i2, j1, j2 in difflib.SequenceMatcher(None, norm[0], norm[1], autojunk=False).get_opcodes():
        if op == "equal" or (op == "replace" and i2 - i1 == j2 - j1):
            out += [(i1 + k, j1 + k) for k in range(i2 - i1)]
    return out


def diff_facts(retail, ours, limit=8):
    """Retail and ours disassembly (lists of instruction text) -> {pairs: [(ours value,
    retail value)], cmp: bool}: the immediates that differ in otherwise equal
    instructions (a lui as the whole constant it builds, a shift also as its power of
    two), and whether a compare or branch differs."""
    pairs, cmp = [], False
    for i, j in aligned_pairs(retail, ours):
        a, b = retail[i], ours[j]
        if a == b:
            continue
        ma, mb = a.split()[0] if a.split() else "", b.split()[0] if b.split() else ""
        if ma in CMP_OPS or mb in CMP_OPS:
            cmp = True
        pa, pb = ins_imm(a), ins_imm(b)
        if not pa or not pb or pa[0] != pb[0] or pa[2] == pb[2]:
            continue
        if pa[0] == "lui":
            if 0x8000 <= pa[2] <= 0x80FF or 0x8000 <= pb[2] <= 0x80FF:
                continue                         # an address: a symbol, not a C constant
            new = [(lui_value(ours, j), lui_value(retail, i))]
        elif pa[0] in ("sll", "srl", "sra"):
            new = [(pb[2], pa[2]), (1 << pb[2], 1 << pa[2])]
        else:
            new = [(pb[2], pa[2])]
        for p in new:
            if p[0] != p[1] and p not in pairs:
                pairs.append(p)
    return {"pairs": pairs[:limit], "cmp": cmp}


def spell(n, hexy):
    return ("0x%X" % n) if hexy else str(n)


def lever_imm(text, fn, facts):
    """A literal of the function equal to ours (or ours +- 1: `<=` compiles as `< v+1`) ->
    retail's value, the same offset kept; `+ v` / `- v` turn into each other when the
    sign changes."""
    sp = fn_span(text, fn)
    if not sp or not facts or not facts["pairs"]:
        return []
    body = text[sp[0]:sp[1]]
    out, seen = [], set()
    for m in LIT.finditer(body):
        mag = int(m.group(2), 0)
        binop = re.search(r"([\w)\]])\s*([+-])\s*$", body[:m.start() + len(m.group(1))])
        if binop:                                # `x - 16`: the value is -16, the edit spans the operator
            start, v = binop.start(2), (-mag if binop.group(2) == "-" else mag)
        else:
            start, v = m.start(), (-mag if m.group(1) else mag)
        hexy = m.group(2).lower().startswith("0x")
        for o, r in facts["pairs"]:
            for d in (0, -1, 1):
                if (v - (o + d)) & 0xFFFFFFFF:
                    continue
                n = r + d
                if hexy and not binop and v >= 0 and n < 0:
                    n &= 0xFFFFFFFF                 # an unsigned spelling stays unsigned
                if binop:
                    new = ("+ " if n >= 0 else "- ") + spell(abs(n), hexy)
                else:
                    new = spell(n, hexy) if n >= 0 else "-" + spell(-n, hexy)
                nb = body[:start] + new + body[m.end():]
                if nb in seen or nb == body:
                    continue
                seen.add(nb)
                out.append(("imm %s->%s" % (body[start:m.end()].strip(), new), text[:sp[0]] + nb + text[sp[1]:]))
    return out


CMP = re.compile(r"(?<=\s)(<=|>=|==|!=|<|>)(?=\s)")
CMP_ALT = {"<": ["<=", ">", ">="], "<=": ["<", ">=", ">"], ">": [">=", "<", "<="],
           ">=": [">", "<=", "<"], "==": ["!="], "!=": ["=="]}


def lever_cmp(text, fn, facts):
    """When a compare or branch differs: every comparison of the function in the other
    relational spellings (the constant next to it is the imm lever's)."""
    sp = fn_span(text, fn)
    if not sp or not facts or not facts["cmp"]:
        return []
    body = text[sp[0]:sp[1]]
    out = []
    for m in CMP.finditer(body):
        for alt in CMP_ALT[m.group(1)]:
            nb = body[:m.start()] + alt + body[m.end():]
            out.append(("cmp %s->%s" % (m.group(1), alt), text[:sp[0]] + nb + text[sp[1]:]))
    return out


LEVERS = {"index": lever_index, "bytes": lever_bytes, "ptype": lever_ptype, "etype": lever_etype,
          "const": lever_const, "swap": lever_swap, "commute": lever_commute, "args": lever_args}
DIFF_LEVERS = {"imm": lever_imm, "cmp": lever_cmp}       # (text, fn, diff_facts) -> variants
ALL_LEVERS = dict(LEVERS, **DIFF_LEVERS)
DEFAULT_LEVERS = "imm,index,bytes,args,ptype,etype,const,swap,commute,cmp"


def wants_facts(names):
    return any(n.strip() in DIFF_LEVERS for n in (names or "").split(","))


def variants(text, fn, names=DEFAULT_LEVERS, facts=None):
    """facts: diff_facts() of this text's compiled code; the diff levers give nothing without."""
    out = []
    for n in names.split(","):
        n = n.strip()
        if n in DIFF_LEVERS:
            out += DIFF_LEVERS[n](text, fn, facts)
        elif n:
            out += LEVERS[n](text, fn)
    return out
