#!/usr/bin/env python3
"""live_probe.py - check proposed struct layouts against the running game, one action at a time.

    live_probe.py                     run every step in probe_steps.json, in order
    live_probe.py --step buy --step head
    live_probe.py --list              the steps and what each one checks
    live_probe.py --show credits      print a region's fields as they are in RAM right now
    live_probe.py --check             MCP health check + call latency, then exit

The matching agents proposed struct layouts from the code alone (credits at 0x80039CA4,
equipped part ids at 0x80031A58, the pad state at 0x801A2550 ...).  Each step here asks
you to do one thing in the game and checks what that did to RAM:

  diff steps     snapshot the watched regions, you do the action (buy a part, swap the
                 head ...), press Enter, snapshot again.  Every changed field is printed,
                 bytes no field covers too (those are new findings).
  sample steps   after a 3 s countdown (time to switch to the game window) the regions are
                 read over and over for a few seconds while you hold a button or keep your
                 hands off; fields that move are printed per phase.

Each expectation gets a verdict: CONFIRMED, REFUTED (with what RAM did instead) or UNCLEAR
(skipped question, region unreadable).  Every verdict and every unexpected change is
appended to re/match/live_probe.tsv
(step  field  addr  expect  before  after  verdict  note  timestamp).

Steps, regions and expectations live in probe_steps.json next to this script: add a region
or a step there, nothing here needs to change.  Expectation kinds:
  changes, same, increases, decreases, delta (value "-price": signed answer to a question),
  one_changes / one_up / one_down (arrays: exactly one element changed / +1 / -1),
  one_bit_set (arrays: exactly one element gained `mask`), only (region: no byte outside
  `fields` changed), contains (region: the answer to `value` appears as ASCII),
  held (sample: the field's usual value in phase `in` is nonzero and differs from idle),
  ticks (sample: changes within phase `in`), steady (sample: does not change within `in`).

The MCP server keeps a single session: starting this script ends any other client's
session (coverage_tag.py, another Claude session).  Agree who owns port 2346 first.
Reads only: it never pauses the game or sets breakpoints.
"""
import argparse
import collections
import datetime
import json
import os
import re
import struct
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from coverage_tag import REPO, URL, Mcp, McpError, clean  # noqa: E402

STEPS = os.path.join(HERE, "probe_steps.json")
OUT = os.path.join(REPO, "re", "match", "live_probe.tsv")
OUT_COLS = ["step", "field", "addr", "expect", "before", "after", "verdict", "note", "timestamp"]
COUNTDOWN = 3.0
INT_FMT = {"u8": "<B", "s8": "<b", "u16": "<H", "s16": "<h", "u32": "<I", "s32": "<i"}


# ---------------------------------------------------------------------------
# layouts
# ---------------------------------------------------------------------------
class Field:
    def __init__(self, name, off, typ):
        m = re.fullmatch(r"(u8|s8|u16|s16|u32|s32|rgb|char)(?:\[(\d+)\])?", typ)
        if not m:
            raise ValueError("bad field type %r for %s" % (typ, name))
        self.name, self.off, self.typ, self.base = name, off, typ, m.group(1)
        n = int(m.group(2)) if m.group(2) else None
        if self.base == "char":                      # char[N] is one string value
            self.esize, self.count = n or 1, None
        else:
            self.esize, self.count = (3 if self.base == "rgb" else struct.calcsize(INT_FMT[self.base])), n
        self.size = self.esize * (self.count or 1)

    def elem(self, raw):
        if self.base == "rgb":
            return tuple(raw)
        if self.base == "char":
            return bytes(raw)
        return struct.unpack(INT_FMT[self.base], raw)[0]

    def value(self, data):
        """Scalar, or a list for arrays, from the region's bytes."""
        if self.count is None:
            return self.elem(data[self.off:self.off + self.esize])
        return [self.elem(data[self.off + i * self.esize:self.off + (i + 1) * self.esize])
                for i in range(self.count)]

    def fmt(self, v):
        if isinstance(v, list):
            return "[%s]" % ",".join(self.fmt(x) for x in v)
        if isinstance(v, tuple):
            return "(%s)" % ",".join(str(x) for x in v)
        if isinstance(v, bytes):
            return fmt_text(v)
        return "0x%08X" % v if self.base == "u32" else str(v)


class Region:
    def __init__(self, name, spec):
        self.name, self.addr, self.size = name, int(spec["addr"], 16), int(spec["size"])
        self.src = spec.get("src", "")
        self.fields = [Field(f[0], int(f[1]), f[2]) for f in spec.get("fields", [])]
        for f in self.fields:
            if f.off + f.size > self.size:
                raise ValueError("%s.%s runs past the region (size %d)" % (name, f.name, self.size))

    def field(self, name):
        for f in self.fields:
            if f.name == name:
                return f
        raise KeyError("%s has no field %s" % (self.name, name))

    def label(self, off):
        """Field label for a byte offset: `head`, `count[17]`, or `+0x2C` when no field covers it."""
        for f in self.fields:
            if f.off <= off < f.off + f.size:
                return f.name if f.count is None else "%s[%d]" % (f.name, (off - f.off) // f.esize)
        return "+0x%X" % off


def fmt_text(b):
    t = b.split(b"\0", 1)[0]
    s = "".join(chr(c) if 32 <= c < 127 else "\\x%02X" % c for c in t)
    return '"%s"' % s


def load_spec(path=STEPS):
    with open(path, encoding="utf-8") as fh:
        spec = json.load(fh)
    regions = {k: Region(k, v) for k, v in spec["regions"].items()}
    steps = spec["steps"]
    for s in steps:
        for r in s.get("watch", []):
            if r not in regions:
                raise ValueError("step %s watches unknown region %s" % (s["id"], r))
        for e in s.get("expect", []):
            target(regions, e)                       # raises on a bad name
    return regions, steps


def target(regions, exp):
    """(region, field or None) an expectation is about."""
    if "region" in exp:
        return regions[exp["region"]], None
    rname, fname = exp["field"].split(".", 1)
    return regions[rname], regions[rname].field(fname)


def exp_text(exp):
    t = exp["is"]
    for k in ("value", "mask", "in"):
        if k in exp:
            t += " %s=%s" % (k, exp[k])
    if "fields" in exp:
        t += " " + ",".join(exp["fields"])
    return t


# ---------------------------------------------------------------------------
# what changed
# ---------------------------------------------------------------------------
def changes(region, before, after):
    """[(label, old, new)] per changed field element, plus raw bytes no field covers."""
    out, covered = [], set()
    for f in region.fields:
        covered.update(range(f.off, f.off + f.size))
        a, b = f.value(before), f.value(after)
        if f.count is None:
            if a != b:
                out.append((f.name, f.fmt(a), f.fmt(b)))
        else:
            out += [("%s[%d]" % (f.name, i), f.fmt(x), f.fmt(y)) for i, (x, y) in enumerate(zip(a, b)) if x != y]
    for off in range(region.size):
        if off not in covered and before[off] != after[off]:
            out.append(("+0x%X" % off, "0x%02X" % before[off], "0x%02X" % after[off]))
    return out


def number(s):
    try:
        return int(str(s).replace(",", "").strip())
    except ValueError:
        return None


def check_diff(exp, regions, before, after, answers):
    """-> (verdict, before text, after text, note) for a diff-step expectation."""
    reg, f = target(regions, exp)
    b, a = before.get(reg.name), after.get(reg.name)
    if b is None or a is None:
        return "UNCLEAR", "", "", "region not read"
    kind = exp["is"]
    if kind == "only":
        keep = set()
        for n in exp["fields"]:
            g = reg.field(n)
            keep.update(range(g.off, g.off + g.size))
        stray = sorted({reg.label(o) for o in range(reg.size) if b[o] != a[o] and o not in keep})
        return ("REFUTED", "", "", "also changed: " + ", ".join(stray)) if stray else ("CONFIRMED", "", "", "")
    if kind == "contains":
        want = answers.get(exp["value"], "")
        if not want:
            return "UNCLEAR", "", "", "no answer for %s" % exp["value"]
        hit = a.lower().find(want.encode("ascii", "replace").lower())
        if hit >= 0:
            return "CONFIRMED", "", fmt_text(a[hit:]), "at %s" % reg.label(hit)
        moved = changes(reg, b, a)
        return "REFUTED", "", "", "not found as ASCII; changed: %s" % (
            "; ".join("%s %s -> %s" % m for m in moved) or "nothing")
    old, new = f.value(b), f.value(a)
    ob, nb = f.fmt(old), f.fmt(new)
    if kind == "changes":
        return ("CONFIRMED" if old != new else "REFUTED"), ob, nb, ""
    if kind == "same":
        return ("CONFIRMED" if old == new else "REFUTED"), ob, nb, ""
    if kind in ("increases", "decreases"):
        ok = new > old if kind == "increases" else new < old
        return ("CONFIRMED" if ok else "REFUTED"), ob, nb, "delta %+d" % (new - old)
    if kind == "delta":
        m = re.fullmatch(r"([+-]?)(\w+)", exp["value"])
        n = number(answers.get(m.group(2), ""))
        if n is None:
            ok = (new < old) if m.group(1) == "-" else (new > old)
            return ("UNCLEAR" if ok else "REFUTED"), ob, nb, "delta %+d (no %s given)" % (new - old, m.group(2))
        want = -n if m.group(1) == "-" else n
        return ("CONFIRMED" if new - old == want else "REFUTED"), ob, nb, "delta %+d, expected %+d" % (new - old, want)
    if kind in ("one_changes", "one_up", "one_down", "one_bit_set"):
        if f.count is None:
            raise ValueError("%s needs an array field" % kind)
        if kind == "one_bit_set":
            mask = int(exp["mask"])
            idx = [i for i, (x, y) in enumerate(zip(old, new)) if not x & mask and y & mask]
        else:
            idx = [i for i, (x, y) in enumerate(zip(old, new)) if x != y]
        if len(idx) != 1:
            return "REFUTED", "", "", "%d element(s) changed%s" % (
                len(idx), (": " + ", ".join(str(i) for i in idx[:12])) if idx else "")
        i = idx[0]
        ob, nb = f.fmt(old[i]), f.fmt(new[i])
        note = "%s[%d]" % (f.name, i)
        if kind == "one_up" and new[i] - old[i] != 1 or kind == "one_down" and old[i] - new[i] != 1:
            return "REFUTED", ob, nb, note + " moved by %+d" % (new[i] - old[i])
        return "CONFIRMED", ob, nb, note
    raise ValueError("unknown expectation %r" % kind)


# ---------------------------------------------------------------------------
# sampling
# ---------------------------------------------------------------------------
def mode(vals):
    """(most common value, its share, distinct count); lists are compared as tuples."""
    keyed = [tuple(v) if isinstance(v, list) else v for v in vals]
    c = collections.Counter(keyed)
    v, n = c.most_common(1)[0]
    return (list(v) if isinstance(vals[0], list) else v), n / len(keyed), len(c)


def check_sample(exp, regions, phases):
    """-> (verdict, before text, after text, note); phases = {label: [{region: bytes}]}."""
    reg, f = target(regions, exp)
    ph = exp.get("in", "idle")
    if ph not in phases:
        return "UNCLEAR", "", "", "phase %s not sampled" % ph
    vals = [f.value(s[reg.name]) for s in phases[ph] if reg.name in s]
    if not vals:
        return "UNCLEAR", "", "", "no samples"
    v, share, distinct = mode(vals)
    kind = exp["is"]
    if kind == "ticks":
        return ("CONFIRMED" if distinct > 1 else "REFUTED"), f.fmt(vals[0]), f.fmt(vals[-1]), "%d value(s)" % distinct
    if kind == "steady":
        return ("CONFIRMED" if distinct == 1 else "REFUTED"), f.fmt(vals[0]), f.fmt(vals[-1]), "%d value(s)" % distinct
    if kind == "held":
        idle = [f.value(s[reg.name]) for s in phases.get("idle", []) if reg.name in s]
        iv = mode(idle)[0] if idle else None
        note = "usual %s in %.0f%% of %d samples" % (f.fmt(v), share * 100, len(vals))
        if iv is None:
            return "UNCLEAR", "", f.fmt(v), note + "; no idle phase"
        ok = v != iv and v != 0 and share >= 0.5
        if ok and isinstance(v, int) and isinstance(iv, int):
            note += "; bits 0x%X" % (v & ~iv)
        return ("CONFIRMED" if ok else "REFUTED"), f.fmt(iv), f.fmt(v), note
    raise ValueError("unknown sample expectation %r" % kind)


def moving(regions, names, phases):
    """[(label, {phase: 'mode (n values)'})] for every field element or loose byte that is not
    one constant value across all samples of all phases."""
    out = []
    for rn in names:
        reg = regions[rn]
        samples = [(p, s[rn]) for p, ss in phases.items() for s in ss if rn in s]
        if not samples:
            continue
        first = samples[0][1]
        diff = {o for _p, d in samples for o in range(reg.size) if d[o] != first[o]}
        labels = []
        for o in sorted(diff):
            lab = reg.label(o)
            if lab not in labels:
                labels.append(lab)
        for lab in labels:
            per = {}
            for p in phases:
                vals = [label_value(reg, lab, d) for q, d in samples if q == p]
                if vals:
                    v, _share, n = mode(vals)
                    per[p] = "%s (%d value%s)" % (v, n, "" if n == 1 else "s")
            out.append(("%s.%s" % (rn, lab), per))
    return out


def label_value(reg, lab, data):
    if lab.startswith("+0x"):
        return "0x%02X" % data[int(lab[1:], 16)]
    m = re.fullmatch(r"(\w+)(?:\[(\d+)\])?", lab)
    f = reg.field(m.group(1))
    v = f.value(data)
    if m.group(2) is not None:
        v = v[int(m.group(2))]
    return f.fmt(v)


# ---------------------------------------------------------------------------
# the session
# ---------------------------------------------------------------------------
def read_tsv_header(path):
    if not os.path.isfile(path):
        return None
    with open(path, encoding="utf-8") as fh:
        return fh.readline().rstrip("\n").split("\t")


def append_rows(path, rows):
    if not rows:
        return
    new = read_tsv_header(path) is None
    with open(path, "a", encoding="utf-8", newline="\n") as fh:
        if new:
            fh.write("\t".join(OUT_COLS) + "\n")
        for r in rows:
            fh.write("\t".join(clean(str(r.get(c, ""))) for c in OUT_COLS) + "\n")


class Probe:
    def __init__(self, mcp, regions, ask=input, say=print, sleep=time.sleep, clock=time.time):
        self.mcp, self.regions = mcp, regions
        self.ask, self.say, self.sleep, self.clock = ask, say, sleep, clock

    def snap(self, names):
        out = {}
        for n in names:
            r = self.regions[n]
            try:
                out[n] = self.mcp.read(r.addr, r.size)
            except McpError as e:
                self.say("  could not read %s at 0x%08X: %s" % (n, r.addr, e))
        return out

    def sample(self, names, seconds):
        t_end = self.clock() + seconds
        got = []
        while self.clock() < t_end:
            got.append(self.snap(names))
            self.sleep(0.02)
        return got

    def run_step(self, step):
        """-> list of result rows (dicts with OUT_COLS keys)."""
        sid, names = step["id"], step.get("watch", [])
        if step["kind"] == "diff":
            before = self.snap(names)
            self.ask("  now %s, then press Enter here: " % step["say"])
            after = self.snap(names)
            answers = {k: self.ask("  %s: " % q).strip() for k, q in step.get("ask", {}).items()}
            rows, expected = self.judge(step, lambda e: check_diff(e, self.regions, before, after, answers))
            note = "; ".join("%s=%s" % kv for kv in answers.items() if kv[1])
            for n in names:
                reg = self.regions[n]
                if n not in before or n not in after:
                    continue
                for lab, old, new in changes(reg, before[n], after[n]):
                    full = "%s.%s" % (n, lab)
                    self.say("  changed %-22s %s -> %s" % (full, old, new))
                    if full.split("[")[0] not in expected:
                        rows.append({"step": sid, "field": full, "addr": "0x%08X" % (reg.addr + offset_of(reg, lab)),
                                     "expect": "-", "before": old, "after": new, "verdict": "seen", "note": note})
            return rows
        phases = {}
        for ph in step["phases"]:
            self.ask("  phase %s: press Enter, switch to the game within %.0f s and %s for %s s: "
                     % (ph["label"], COUNTDOWN, ph["say"], ph.get("seconds", 3)))
            self.sleep(COUNTDOWN)
            self.say("  sampling ...")
            phases[ph["label"]] = self.sample(names, float(ph.get("seconds", 3)))
            self.say("  %d sample(s)" % len(phases[ph["label"]]))
        rows, expected = self.judge(step, lambda e: check_sample(e, self.regions, phases))
        for full, per in moving(self.regions, names, phases):
            self.say("  moving  %-22s %s" % (full, "  ".join("%s: %s" % kv for kv in per.items())))
            if full.split("[")[0] not in expected:
                rn, lab = full.split(".", 1)
                reg = self.regions[rn]
                rows.append({"step": sid, "field": full, "addr": "0x%08X" % (reg.addr + offset_of(reg, lab)),
                             "expect": "-", "before": "", "after": "", "verdict": "seen",
                             "note": "; ".join("%s %s" % kv for kv in per.items())})
        return rows

    def judge(self, step, fn):
        rows, expected = [], set()
        for e in step.get("expect", []):
            reg, f = target(self.regions, e)
            try:
                verdict, b, a, note = fn(e)
            except (ValueError, KeyError) as err:
                verdict, b, a, note = "UNCLEAR", "", "", str(err)
            name = "%s.%s" % (reg.name, f.name) if f else reg.name
            if f:
                expected.add(name)
            self.say("  %-9s %-22s %-24s %s%s" % (verdict, name, exp_text(e),
                                                  ("%s -> %s  " % (b, a)) if (b or a) else "", note))
            rows.append({"step": step["id"], "field": name, "addr": "0x%08X" % (reg.addr + (f.off if f else 0)),
                         "expect": exp_text(e), "before": b, "after": a, "verdict": verdict, "note": note})
        return rows, expected


def offset_of(reg, lab):
    if lab.startswith("+0x"):
        return int(lab[1:], 16)
    m = re.fullmatch(r"(\w+)(?:\[(\d+)\])?", lab)
    f = reg.field(m.group(1))
    return f.off + (int(m.group(2)) * f.esize if m.group(2) else 0)


def show(probe, name):
    reg = probe.regions[name]
    data = probe.snap([name]).get(name)
    if data is None:
        return 2
    print("%s @0x%08X (%d bytes)  %s" % (name, reg.addr, reg.size, reg.src))
    for f in reg.fields:
        print("  +0x%-4X %-12s %-9s %s" % (f.off, f.name, f.typ, f.fmt(f.value(data))))
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--step", action="append", help="run only this step id (repeatable)")
    ap.add_argument("--list", action="store_true", help="list the steps and exit")
    ap.add_argument("--show", metavar="REGION", action="append", help="print a region's fields now and exit")
    ap.add_argument("--check", action="store_true", help="health check: server info and call latency")
    ap.add_argument("--steps", default=STEPS, help="step file (default probe_steps.json)")
    ap.add_argument("--out", default=OUT, help="results TSV to append to ('' = none)")
    ap.add_argument("--url", default=URL)
    args = ap.parse_args(argv)

    regions, steps = load_spec(args.steps)
    if args.list:
        for s in steps:
            print("%-8s %-6s at %s: %s" % (s["id"], s["kind"], s["where"],
                                           s.get("say") or " / ".join(p["say"] for p in s["phases"])))
            for e in s.get("expect", []):
                reg, f = target(regions, e)
                print("           expect %s.%s %s   [%s]" % (reg.name, f.name if f else "*", exp_text(e), reg.src))
        return 0
    if args.step:
        unknown = set(args.step) - {s["id"] for s in steps}
        if unknown:
            print("live_probe: unknown step(s) %s" % ", ".join(sorted(unknown)), file=sys.stderr)
            return 3
        steps = [s for s in steps if s["id"] in args.step]

    mcp = Mcp(args.url)
    try:
        info = mcp.initialize()
    except McpError as e:
        print("live_probe: %s" % e, file=sys.stderr)
        return 2
    print("MCP: %s %s" % (info.get("name", "?"), info.get("version", "")))
    probe = Probe(mcp, regions)
    if args.check:
        t = time.time()
        got = probe.snap(list(regions))
        print("read %d/%d region(s) in %.0f ms" % (len(got), len(regions), (time.time() - t) * 1000))
        return 0 if got else 2
    if args.show:
        return max(show(probe, n) for n in args.show)

    tally = collections.Counter()
    try:
        for s in steps:
            print("\n== %s: %s\n   be at: %s" % (s["id"], s.get("say") or s["kind"], s["where"]))
            while True:
                c = probe.ask("  Enter = start, s = skip, q = quit: ").strip().lower()
                if c in ("s", "q"):
                    break
                stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
                rows = probe.run_step(s)
                for r in rows:
                    r["timestamp"] = stamp
                    tally[r["verdict"]] += 1
                if args.out:
                    append_rows(args.out, rows)
                c = probe.ask("  Enter = next step, a = do this step again, q = quit: ").strip().lower()
                if c != "a":
                    break
            if c == "q":
                break
    except (KeyboardInterrupt, EOFError):
        print("\nstopping")
    except McpError as e:
        print("live_probe: %s" % e, file=sys.stderr)
        return 2
    print("\n%s%s" % (", ".join("%d %s" % (n, v) for v, n in sorted(tally.items())) or "no results",
                      ("; appended to %s" % os.path.relpath(args.out, REPO).replace("\\", "/")) if args.out and tally else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
