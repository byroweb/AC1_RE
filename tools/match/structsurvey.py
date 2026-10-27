#!/usr/bin/env python3
"""structsurvey.py - find the struct layouts matched sources declare locally.

Matching agents were told not to edit include/ac1, so a layout they recover goes into
the matched C file as a local typedef.  This tool reads every re/match/**/*.c, lays
out each local struct/union with the MIPS (GCC 2.7.2) rules, and reports

    by-address   structs bound to a global: `extern T D_XXXXXXXX;`, `extern T *D_...;`,
                 `(T *)0xXXXXXXXX` / `(T *)&D_XXXXXXXX` casts, and names from
                 include/ac1/symbols.tsv or ac1_markup.tsv
    shared       the same layout (field offsets and widths, names ignored) declared
                 in two or more files

    structsurvey.py report [--min-files N] [--addr 0x1F800000 ...] [-o FILE]
    structsurvey.py fields TYPE-OR-ADDR ...    every local declaration of one name or
                                               global, field by field, with its file
    structsurvey.py json [-o FILE]             everything, for other tools

Offsets are computed, not read from the comments; a field whose own `+0xNN` comment
disagrees with the computed offset is flagged with `!`.  Sizes of types from
include/ac1 come from their AC1_ASSERT_SIZE lines.  Pointers count as 4 bytes, so a
pointer-to-struct field never needs the pointee's layout.
"""
import argparse
import collections
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
MATCH = os.path.join(ROOT, "re", "match")
AC1 = os.path.join(ROOT, "include", "ac1")

# (size, align)
PRIM = {
    "u8": (1, 1), "s8": (1, 1), "char": (1, 1), "unsigned char": (1, 1), "signed char": (1, 1),
    "u16": (2, 2), "s16": (2, 2), "short": (2, 2), "unsigned short": (2, 2),
    "u32": (4, 4), "s32": (4, 4), "int": (4, 4), "unsigned int": (4, 4), "unsigned": (4, 4),
    "long": (4, 4), "unsigned long": (4, 4), "float": (4, 4), "double": (8, 8),
    "long long": (8, 8), "unsigned long long": (8, 8), "s64": (8, 8), "u64": (8, 8),
    "void": (1, 1),
    "SVECTOR": (8, 2), "VECTOR": (16, 4), "MATRIX": (32, 4), "DVECTOR": (4, 2),
    "RECT": (8, 2), "CVECTOR": (4, 1), "P_TAG": (4, 4),
}
QUALS = {"volatile", "const", "register", "static", "extern", "signed"}


def strip_comments(text):
    """Comments -> spaces (keeps offsets), returns (code, list of (pos, comment))."""
    out, notes, i = [], [], 0
    for m in re.finditer(r"/\*.*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'", text, re.S):
        if m.group().startswith("/*"):
            out.append(text[i:m.start()])
            out.append(re.sub(r"[^\n]", " ", m.group()))
            notes.append((m.start(), m.group()[2:-2].strip()))
        else:
            out.append(text[i:m.end()])
        i = m.end()
    out.append(text[i:])
    return "".join(out), notes


def header_sizes():
    """(size, align) of every include/ac1 type: laid out from the header (types.h
    first), with the AC1_ASSERT_SIZE value winning where there is one."""
    asserted = {}
    files = sorted(fn for fn in os.listdir(AC1) if fn.endswith(".h") and fn != "symbols.h")
    for fn in files:
        with open(os.path.join(AC1, fn), encoding="utf-8") as fh:
            for m in re.finditer(r"AC1_ASSERT_SIZE\(\s*(\w+)\s*,\s*(0x[0-9A-Fa-f]+|\d+)\s*\)", fh.read()):
                asserted[m.group(1)] = int(m.group(2), 0)
    sizes = dict((n, (v, 4 if v % 4 == 0 else (2 if v % 2 == 0 else 1))) for n, v in asserted.items())
    files.sort(key=lambda fn: fn != "types.h")
    for fn in files:
        u = Unit(os.path.join(AC1, fn), fn, sizes, {})
        for name, lay in u.types.items():
            if name.startswith(("struct ", "union ")):
                continue
            if name in asserted:
                sizes[name] = (asserted[name], lay["align"])
            elif lay["ok"]:
                sizes[name] = (lay["size"], lay["align"])
    return sizes


def global_names():
    """name -> address from symbols.tsv and ac1_markup.tsv."""
    names = {}
    for path in (os.path.join(AC1, "symbols.tsv"), os.path.join(ROOT, "ac1_markup.tsv")):
        if not os.path.exists(path):
            continue
        with open(path, encoding="utf-8") as fh:
            for line in fh:
                if line.startswith("#") or not line.strip():
                    continue
                p = line.rstrip("\r\n").split("\t")
                if len(p) >= 2 and re.fullmatch(r"0x[0-9A-Fa-f]{8}", p[0].strip()):
                    names.setdefault(p[1].strip(), int(p[0], 16))
    return names


def match_brace(code, i):
    """code[i] == '{'; index of the matching '}'."""
    depth = 0
    for j in range(i, len(code)):
        if code[j] == "{":
            depth += 1
        elif code[j] == "}":
            depth -= 1
            if depth == 0:
                return j
    return -1


def split_top(s, sep):
    parts, depth, cur = [], 0, []
    for ch in s:
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == sep and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    parts.append("".join(cur))
    return parts


def cexpr(s, macros):
    s = s.strip()
    for _ in range(4):
        s = re.sub(r"\b[A-Za-z_]\w*\b", lambda m: macros.get(m.group(), m.group()), s)
    s = re.sub(r"(0x[0-9A-Fa-f]+|\d+)[uUlL]+", r"\1", s)
    if not re.fullmatch(r"[0-9A-Fa-fx()+\-*/<>| ]+", s):
        return None
    try:
        return int(eval(s.replace("/", "//"), {"__builtins__": {}}))
    except Exception:
        return None


class Unit:
    """One file's local types."""

    def __init__(self, path, rel, hsizes, gnames):
        self.path, self.rel = path, rel
        self.hsizes, self.gnames = hsizes, gnames
        with open(path, encoding="utf-8", errors="replace") as fh:
            self.raw = fh.read()
        self.code, self.notes = strip_comments(self.raw)
        self.macros = {}
        for m in re.finditer(r"^[ \t]*#[ \t]*define[ \t]+(\w+)[ \t]+(.+)$", self.code, re.M):
            self.macros[m.group(1)] = m.group(2).strip()
        self.types = {}       # typedef name or "struct Tag" -> layout dict
        self.alias = {}       # typedef name -> type string (non-struct typedefs)
        self.order = []
        self.parse()

    # -- layout --------------------------------------------------------------
    def size_of(self, tname):
        tname = " ".join(w for w in tname.split() if w not in QUALS) or "int"
        if tname in self.types:
            t = self.types[tname]
            return t["size"], t["align"]
        if tname in self.alias:
            return self.size_of(self.alias[tname])
        if tname in PRIM:
            return PRIM[tname]
        if tname in self.hsizes:
            return self.hsizes[tname]
        return None

    def note_at(self, start, end):
        best = None
        for pos, txt in self.notes:
            if start <= pos < end:
                best = txt if best is None else best
        return best

    def comment_after(self, end):
        """first comment on the same line after `end`, or inside the decl text."""
        line_end = self.raw.find("\n", end)
        line_end = len(self.raw) if line_end < 0 else line_end
        for pos, txt in self.notes:
            if end <= pos < line_end:
                return txt
        return None

    def layout(self, kind, body, body_start):
        fields, off, align, ok = [], 0, 1, True
        pos = 0
        for decl in split_top(body, ";"):
            dstart = body_start + pos
            pos += len(decl) + 1
            d = decl.strip()
            if not d or d.startswith("#"):
                continue
            note = self.comment_after(body_start + pos - 1)
            # nested struct/union
            m = re.match(r"(struct|union)\s*(\w*)\s*\{", d)
            if m:
                ob = decl.index("{")
                cb = match_brace(decl, ob)
                inner = self.layout(m.group(1), decl[ob + 1:cb], dstart + ob + 1)
                if m.group(2):
                    self.types["%s %s" % (m.group(1), m.group(2))] = inner
                tname = None
                decls = decl[cb + 1:]
                base = inner
            else:
                base = None
                m2 = re.match(r"((?:(?:volatile|const|unsigned|signed|struct|union|enum|short|long|int|char)\s+)*)(\w+)(.*)$", d, re.S)
                if not m2:
                    ok = False
                    continue
                spec, last, decls = m2.groups()
                tname = (spec + last).strip()
                # `unsigned x`, `short x` etc.: the last word is the declarator
                words = tname.split()
                if words[-1] not in PRIM and words[-1] not in self.types and words[-1] not in self.alias \
                        and words[-1] not in self.hsizes and len(words) > 1 \
                        and " ".join(words[:-1]).replace("struct ", "").replace("union ", "") in \
                        ("unsigned", "signed", "short", "long", "unsigned short", "unsigned long", "unsigned int", "int", "char"):
                    decls = words[-1] + decls
                    tname = " ".join(words[:-1])
            for di, dd in enumerate(split_top(decls, ",")):
                dd = dd.strip()
                if not dd:
                    if base is not None:   # anonymous member struct
                        dd = "_anon"
                    else:
                        continue
                bits = None
                if ":" in dd and "?" not in dd:
                    dd, b = dd.split(":", 1)
                    bits = cexpr(b, self.macros)
                    dd = dd.strip() or "_bits"
                fp = re.match(r"\(\s*\*\s*(\w+)\s*\)\s*\(", dd)
                if fp:
                    name, ptr, dims = fp.group(1), 1, []
                    dd2 = dd
                else:
                    ptr = dd.count("*")
                    dd2 = dd.replace("*", " ").strip()
                    mm = re.match(r"(\w+)\s*((?:\[[^\]]*\]\s*)*)$", dd2)
                    if not mm:
                        ok = False
                        continue
                    name = mm.group(1)
                    dims = re.findall(r"\[([^\]]*)\]", mm.group(2))
                if ptr:
                    esz, eal, tdesc = 4, 4, (tname or "struct") + " " + "*" * ptr
                elif base is not None:
                    esz, eal, tdesc = base["size"], base["align"], base["kind"] + " {...}"
                else:
                    sa = self.size_of(tname)
                    if sa is None:
                        ok = False
                        esz, eal = 4, 4
                        tdesc = tname + "?"
                    else:
                        esz, eal = sa
                        tdesc = tname
                n = 1
                for dim in dims:
                    v = cexpr(dim, self.macros) if dim.strip() else 0
                    if v is None:
                        ok = False
                        v = 1
                    n *= v
                size = esz * n
                if bits is not None:
                    # rare; treat as packed into the unit of its type, no attempt at bit offsets
                    ok = ok and True
                if kind == "struct":
                    off = (off + eal - 1) // eal * eal
                    fo = off
                    off += size
                else:
                    fo = 0
                    off = max(off, size)
                align = max(align, eal)
                f = {"off": fo, "size": size, "name": name, "type": tdesc + "".join("[%s]" % x for x in dims)}
                c = note if di == 0 else None   # `s16 x, y;  /* +0x04 */` describes x
                if c:
                    f["note"] = c
                    mo = re.search(r"\+\s*(0x[0-9A-Fa-f]+|\d+)", c)
                    if mo and int(mo.group(1), 0) != fo and kind == "struct":
                        f["mismatch"] = int(mo.group(1), 0)
                if base is not None and not ptr:
                    f["sub"] = base["fields"]
                fields.append(f)
        size = (off + align - 1) // align * align
        return {"kind": kind, "size": size, "align": align, "fields": fields, "ok": ok}

    # -- parse ---------------------------------------------------------------
    def parse(self):
        code = self.code
        i = 0
        pat = re.compile(r"\b(typedef\s+)?(struct|union)\s*(\w*)\s*\{")
        while True:
            m = pat.search(code, i)
            if not m:
                break
            # skip struct bodies inside functions / initialisers: require top level
            if code.count("{", 0, m.start()) != code.count("}", 0, m.start()):
                i = m.end()
                continue
            ob = m.end() - 1
            cb = match_brace(code, ob)
            if cb < 0:
                break
            lay = self.layout(m.group(2), code[ob + 1:cb], ob + 1)
            lay["line"] = code.count("\n", 0, m.start()) + 1
            lay["head"] = self.note_at(m.start(), ob + 200 if code.find("\n", ob) < 0 else code.find("\n", ob) + 1)
            semi = code.find(";", cb)
            tail = code[cb + 1:semi]
            names = []
            if m.group(3):
                names.append("%s %s" % (m.group(2), m.group(3)))
            if m.group(1):
                for dd in split_top(tail, ","):
                    dd = dd.strip()
                    if dd and "*" not in dd and "[" not in dd and re.fullmatch(r"\w+", dd):
                        names.append(dd)
            for n in names:
                self.types[n] = lay
            if names:
                lay["names"] = names
                self.order.append(lay)
            elif tail.strip():
                # struct {..} var;  -- anonymous object definition
                lay["names"] = ["<anon %s>" % tail.strip()]
                self.order.append(lay)
                self.bind_var(tail.strip(), lay)
            i = semi + 1 if semi > 0 else cb + 1
        # plain typedefs (typedef T name;)
        for m in re.finditer(r"\btypedef\s+([\w\s]+?)\s+(\w+)\s*;", code):
            self.alias.setdefault(m.group(2), m.group(1))

    # -- address bindings ----------------------------------------------------
    def bindings(self):
        """[(addr, typename, how)]"""
        out = []
        code = self.code
        tnames = set(n for t in self.order for n in t["names"])
        tnames |= set("struct " + n.split()[1] for n in list(tnames) if n.startswith("struct "))

        def tn(s):
            s = " ".join(w for w in s.split() if w not in QUALS)
            return s if s in self.types else None

        for m in re.finditer(r"\bextern\s+([\w\s]+?)\s*(\**)\s*(\w+)\s*((?:\[[^\]]*\])*)\s*;", code):
            t = tn(m.group(1))
            if not t:
                continue
            addr = self.addr_of(m.group(3))
            if addr is not None:
                how = "extern " + ("*" * len(m.group(2))) + ("[]" if m.group(4) else "")
                out.append((addr, t, how.strip()))
        for m in re.finditer(r"\(\s*((?:struct\s+|union\s+)?\w+)\s*\*\s*\)\s*\(?\s*(0x[0-9A-Fa-f]{8}|&\s*\w+)", code):
            t = tn(m.group(1))
            if not t:
                continue
            a = m.group(2)
            addr = int(a, 16) if a.startswith("0x") else self.addr_of(a.lstrip("& \t"))
            if addr is not None:
                out.append((addr, t, "cast"))
        return sorted(set(out))

    def bind_var(self, tail, lay):
        pass

    def addr_of(self, name):
        m = re.fullmatch(r"[DF]_([0-9A-Fa-f]{8})", name)
        if m:
            return int(m.group(1), 16)
        return self.gnames.get(name)


def unit_of(rel):
    return rel.split("/")[0]


def load_all():
    hs, gn = header_sizes(), global_names()
    units = []
    for d in sorted(os.listdir(MATCH)):
        p = os.path.join(MATCH, d)
        if not os.path.isdir(p):
            continue
        for fn in sorted(os.listdir(p)):
            if fn.endswith(".c"):
                units.append(Unit(os.path.join(p, fn), d + "/" + fn, hs, gn))
    return units


def sig(lay):
    """layout signature with names dropped: offsets, sizes, coarse kind."""
    def k(f):
        t = f["type"]
        if "*" in t:
            return "p"
        return "x"
    return (lay["kind"], lay["size"], tuple((f["off"], f["size"], k(f)) for f in lay["fields"]
                                              if not re.fullmatch(r"(unk|pad|_?pad|unused|u|p)[0-9A-Fa-fx_]*", f["name"])))


def fmt_fields(lay, indent="  ", base=0):
    out = []
    for f in lay["fields"]:
        if re.fullmatch(r"(unk|pad|_pad|unused)[0-9A-Fa-fx_]*", f["name"]) and "sub" not in f:
            continue
        mm = " !comment says +0x%X" % f["mismatch"] if "mismatch" in f else ""
        out.append("%s+0x%03X %-4s %-22s %s%s" % (indent, base + f["off"], hex(f["size"])[2:], f["type"], f["name"], mm))
        if "sub" in f and len(out) < 400:
            out += fmt_fields({"fields": f["sub"]}, indent + "    ", base + f["off"])
    return out


def cmd_report(args):
    units = load_all()
    by_addr = collections.defaultdict(list)
    for u in units:
        for addr, t, how in u.bindings():
            by_addr[addr].append((u, t, how))
    lines = ["# structsurvey report: %d matched files, %d local struct/union definitions" %
             (len(units), sum(len(u.order) for u in units)), ""]
    lines.append("## Structs bound to a global address (%d addresses)" % len(by_addr))
    lines.append("")
    want = set(int(a, 16) for a in (args.addr or []))
    for addr in sorted(by_addr):
        rows = by_addr[addr]
        files = sorted(set(u.rel for u, _, _ in rows))
        if want and addr not in want:
            continue
        if not want and len(files) < args.min_files:
            continue
        lines.append("### 0x%08X  (%d files)" % (addr, len(files)))
        for u, t, how in rows:
            lay = u.types[t]
            lines.append("- %s: `%s` %s, 0x%X bytes%s" % (u.rel, t, how, lay["size"], "" if lay["ok"] else " (layout incomplete)"))
        # widest layout in full
        u, t, how = max(rows, key=lambda r: (len(r[0].types[r[1]]["fields"]), r[0].types[r[1]]["size"]))
        lines.append("  widest: %s `%s`" % (u.rel, t))
        lines += fmt_fields(u.types[t], "    ")
        lines.append("")
    if not want:
        groups = collections.defaultdict(list)
        for u in units:
            seen = set()
            for lay in u.order:
                s = sig(lay)
                if len(s[2]) < 2 or id(lay) in seen:
                    continue
                seen.add(id(lay))
                groups[s].append((u, lay))
        shared = [(s, g) for s, g in groups.items() if len(set(u.rel for u, _ in g)) >= max(2, args.min_files)]
        shared.sort(key=lambda sg: (-len(set(u.rel for u, _ in sg[1])), -sg[0][1]))
        lines.append("## Same layout in two or more files (%d groups)" % len(shared))
        lines.append("")
        for s, g in shared:
            names = collections.Counter(n for _, lay in g for n in lay["names"])
            files = sorted(set(u.rel for u, _ in g))
            lines.append("### %s 0x%X, %d fields, %d files: %s" % (s[0], s[1], len(s[2]), len(files),
                                                                  ", ".join("%s x%d" % kv for kv in names.most_common(4))))
            lines.append("  files: " + ", ".join(files[:12]) + (" ..." if len(files) > 12 else ""))
            lines += fmt_fields(g[0][1], "    ")
            lines.append("")
    text = "\n".join(lines) + "\n"
    if args.o:
        with open(args.o, "w", encoding="utf-8") as fh:
            fh.write(text)
    else:
        sys.stdout.write(text)


def cmd_fields(args):
    units = load_all()
    for key in args.keys:
        addr = int(key, 16) if key.lower().startswith("0x") else None
        print("=== %s" % key)
        for u in units:
            hits = []
            if addr is not None:
                hits = [(t, how) for a, t, how in u.bindings() if a == addr]
            else:
                hits = [(n, "") for n in u.types if n == key or n == "struct " + key]
            for t, how in sorted(set(hits)):
                lay = u.types[t]
                print("-- %s:%d `%s` %s size 0x%X%s" % (u.rel, lay.get("line", 0), t, how, lay["size"],
                                                     "" if lay["ok"] else " (incomplete)"))
                print("\n".join(fmt_fields(lay, "    ")))


def flat_fields(fields, base=0):
    for f in fields:
        if "sub" in f and not re.search(r"\[", f["type"]):
            for g in flat_fields(f["sub"], base + f["off"]):
                yield g
        else:
            yield dict(f, off=base + f["off"])


def cmd_merge(args):
    """one row per (offset, width): every name the files gave it, and how many files.
    Rows that overlap a different-width row are marked `~`: a conflict to settle."""
    units = load_all()
    for key in args.keys:
        addr = int(key, 16)
        rows = collections.defaultdict(lambda: collections.defaultdict(set))
        sizes = collections.Counter()
        nfiles = set()
        for u in units:
            for a, t, how in u.bindings():
                if a != addr:
                    continue
                lay = u.types[t]
                if how.startswith("extern *") and args.deref is False:
                    continue
                nfiles.add(u.rel)
                sizes[lay["size"]] += 1
                for f in flat_fields(lay["fields"]):
                    if re.fullmatch(r"(unk|pad|_pad|unused|b|h|w|k|x)?[0-9A-Fa-f]{1,3}|(unk|pad|_pad)\w*", f["name"]):
                        nm = "?"
                    else:
                        nm = f["name"]
                    rows[(f["off"], f["size"])][(f["type"].split("[")[0].strip(), nm)].add(u.rel)
        print("=== 0x%08X: %d files, struct sizes %s" % (addr, len(nfiles),
              ", ".join("0x%X x%d" % kv for kv in sorted(sizes.items()))))
        keys = sorted(rows)
        for i, (off, size) in enumerate(keys):
            named = lambda k: any(nm != "?" for _, nm in rows[k])
            over = named((off, size)) and any((o != off or s != size) and named((o, s))
                                              for o, s in keys if o < off + size and off < o + s)
            names = sorted(rows[(off, size)].items(), key=lambda kv: -len(kv[1]))
            n = len(set().union(*rows[(off, size)].values()))
            print("%s+0x%03X %-4X %2d  %s" % ("~" if over else " ", off, size, n,
                  "; ".join("%s %s x%d" % (t, nm, len(fs)) for (t, nm), fs in names)))


def typed_globals():
    """address -> 'name (C type)' from include/ac1/symbols.tsv."""
    out = {}
    with open(os.path.join(AC1, "symbols.tsv"), encoding="utf-8") as fh:
        for line in fh:
            p = line.rstrip("\r\n").split("\t")
            if len(p) == 4 and p[0].startswith("0x") and "(" not in p[2]:
                out.setdefault(int(p[0], 16), "`%s` %s" % (p[1], p[2]))
    return out


def short_fields(rows, limit):
    """'+off name' for the offsets most files name, in offset order."""
    best = []
    for (off, size), names in rows.items():
        named = [(nm, fs) for (t, nm), fs in names.items() if nm != "?"]
        if not named:
            continue
        nfiles = len(set().union(*[fs for _, fs in named]))
        nm = max(named, key=lambda kv: len(kv[1]))[0]
        best.append((nfiles, off, nm))
    keep = sorted(best, key=lambda b: (-b[0], b[1]))[:limit]
    return ", ".join("+%X %s" % (off, nm) for _, off, nm in sorted(keep, key=lambda b: b[1]))


def cmd_table(args):
    """markdown tables: structs bound to an address, then layouts shared by 2+ files."""
    units = load_all()
    typed = typed_globals()
    by_addr = collections.defaultdict(list)
    for u in units:
        for a, t, how in u.bindings():
            by_addr[a].append((u, t, how))
    out = ["| address | files | units | local names | sizes | most-named fields | shared type |",
           "| --- | --: | --- | --- | --- | --- | --- |"]
    for addr in sorted(by_addr, key=lambda a: (-len(set(r[0].rel for r in by_addr[a])), a)):
        rs = by_addr[addr]
        files = set(u.rel for u, _, _ in rs)
        if len(files) < args.min_files:
            continue
        rows = collections.defaultdict(lambda: collections.defaultdict(set))
        for u, t, how in rs:
            for f in flat_fields(u.types[t]["fields"]):
                nm = "?" if re.fullmatch(r"(unk|pad|_pad|unused|b|h|w|k|x|u|m|f)?[0-9A-Fa-f]{1,4}|(unk|pad|_pad)\w*",
                                         f["name"]) else f["name"]
                rows[(f["off"], f["size"])][(f["type"], nm)].add(u.rel)
        names = collections.Counter(t for _, t, _ in rs)
        sizes = sorted(set(u.types[t]["size"] for u, t, _ in rs))
        ptr = all(h.startswith("extern *") for _, _, h in rs)
        out.append("| 0x%08X%s | %d | %s | %s | %s | %s | %s |" % (
            addr, " (ptr)" if ptr else "", len(files),
            " ".join(sorted(set(unit_of(f) for f in files), key=lambda x: (len(x), x)))[:60],
            ", ".join("%s%s" % (n, "" if c == 1 else " x%d" % c) for n, c in names.most_common(4))
            + (", ..." if len(names) > 4 else ""),
            " ".join("0x%X" % s for s in sizes[:4]) + (" ..." if len(sizes) > 4 else ""),
            short_fields(rows, 6).replace("|", "/"),
            typed.get(addr, "")))
    out += ["", "| layout | files | units | local names | named fields |", "| --- | --: | --- | --- | --- |"]
    bound = set((u.rel, t) for rs in by_addr.values() for u, t, _ in rs)
    groups = collections.defaultdict(list)
    for u in units:
        for lay in u.order:
            s = sig(lay)
            if len(s[2]) >= 2 and not any((u.rel, n) in bound for n in lay["names"]):
                groups[s].append((u, lay))
    shared = [(s, g) for s, g in groups.items() if len(set(u.rel for u, _ in g)) >= max(2, args.min_files)]
    shared.sort(key=lambda sg: (-len(set(u.rel for u, _ in sg[1])), -sg[0][1]))
    for s, g in shared:
        files = set(u.rel for u, _ in g)
        names = collections.Counter(n for _, lay in g for n in lay["names"] if not n.startswith(("struct ", "union ")))
        fl = [f["name"] for f in g[0][1]["fields"]
              if not re.fullmatch(r"(unk|pad|_pad|unused)[0-9A-Fa-fx_]*", f["name"])]
        out.append("| %s 0x%X | %d | %s | %s | %s |" % (
            s[0], s[1], len(files), " ".join(sorted(set(unit_of(f) for f in files), key=lambda x: (len(x), x)))[:60],
            ", ".join("%s%s" % (n, "" if c == 1 else " x%d" % c) for n, c in names.most_common(4))
            + (", ..." if len(names) > 4 else ""),
            ", ".join(fl[:8]) + (", ..." if len(fl) > 8 else "")))
    text = "\n".join(out) + "\n"
    if args.o:
        with open(args.o, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
    else:
        sys.stdout.write(text)


def cmd_json(args):
    units = load_all()
    out = []
    for u in units:
        out.append({"file": u.rel,
                    "types": [dict(lay, names=lay["names"]) for lay in u.order],
                    "bindings": [{"addr": "0x%08X" % a, "type": t, "how": h} for a, t, h in u.bindings()]})
    text = json.dumps(out, indent=1)
    if args.o:
        with open(args.o, "w", encoding="utf-8") as fh:
            fh.write(text)
    else:
        sys.stdout.write(text)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("report")
    r.add_argument("--min-files", type=int, default=1)
    r.add_argument("--addr", action="append")
    r.add_argument("-o")
    f = sub.add_parser("fields")
    f.add_argument("keys", nargs="+")
    g = sub.add_parser("merge")
    g.add_argument("keys", nargs="+")
    g.add_argument("--no-deref", dest="deref", action="store_false",
                   help="skip `extern T *D_X` bindings (a pointer AT the address, not the struct)")
    t = sub.add_parser("table")
    t.add_argument("--min-files", type=int, default=1)
    t.add_argument("-o")
    j = sub.add_parser("json")
    j.add_argument("-o")
    args = ap.parse_args()
    {"report": cmd_report, "fields": cmd_fields, "merge": cmd_merge, "table": cmd_table, "json": cmd_json}[args.cmd](args)


if __name__ == "__main__":
    main()
