#!/usr/bin/env python3
"""
xa_refs.py — which TALK radio lines does the game's data actually reference?

Stage 8 of the voice pipeline: the static cross-check for the reclaimable-space
report (reclaimable.py, docs/AUDIO_PIPELINE.md §7). The chain, all CONFIRMED on SLUS-01323 by reading the
mission overlay (`disc_map/overlays/ovl202_mission.bin`, load base 0x8004ADA0):

  mission script chunk 4            `play_sound <snd>`  (tools/mission/mission_script.py)
    -> FUN_800529F0(0, 0, snd)      looks up u16 table[snd] in the SOUND-SEQUENCE BLOB
    -> sequence interpreter         0x800532A0: the sequence is ASCII text — opcode
       letter + decimal operand (operand parser 0x80052350) — e.g. `M3X40N`
    -> opcode `X`/`x` <n>           calls FUN_8004DB18(n + 0x30): the XA line player,
                                    id >> 4 = XA file table index (3..8 = TALK00..05),
                                    id & 15 = channel  =>  n = 16 * talk_file + channel
       opcode `V`/`v` <n>           same player with 0x90 + n (+ mode << 6): the CPU /
                                    arena music tracks, not radio
       other letters (M, N, T, B, H, W ...) do not reach the XA player from this
       interpreter (jump table at 0x8004B0A8: only X/x and V/v do).

The sequence blob lives in FDAT.T entry 200 (`disc_map/extracted/FDAT_T/entries/
200.bin`, from tools/extract/extract_t.py --dump-entries): a 20-byte header
(u32 0x14 = table offset, ...), then u16 offsets (from the blob start) of ~200
sequences; sequence text = opcodes, then the line's SUBTITLE in the game's font
encoding (A-Z = 0xAA.., a-z = 0xC4.., 0-9 = 0xA0.., '.' 0x8E, ',' 0x8F, '?' 0x90,
'!' 0x91, ':' 0x98, ''' 0x9D). The entry holds three such blobs; they are
parsed independently and the references unioned (they differ slightly).

Sanity anchors (transcript == subtitle): mission 49 plays sequences 91 and 94 =
`X74` / `X77` = TALK04 ch10 "Your fate is sealed." / ch13 "Go back. It is not
too late."; mission 27 plays 57 = `X40` = TALK02 ch08 "With your skill ...".

What this proves and what it does not: a line whose `X n` appears in a sequence
that some mission script plays is **referenced**; a line with no `X n` anywhere,
or only in sequences no script plays, is **statically unreferenced**. Code can
still call FUN_800529F0 / FUN_8004DB18 with constants (the mission-result
jingles do: sequence 101), the mode1/mode2 overlays and the arena are not
scanned, and `call_db18` operands in scripts are small state values, not line
ids. So "unreferenced" means "nothing found", not "never played".

Writes <out>/xa_line_refs.json (local — it carries the subtitle text):
  { "lines": { "<n>": {xa_id, file, channel, sequences:[..], missions:[..], text} },
    "sequences": { "<snd>": {ops, xa_lines, music, text, missions} },
    "missions": { "<m>": {play_sound:[..], call_db18:[..]} }, "blobs": [...] }

Usage:
  tools/audio/.venv/Scripts/python tools/audio/xa_refs.py [--fdat-entries DIR] [--out DIR] [--print]
"""
import argparse
import glob
import json
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "mission"))
import ac1audio as A  # noqa: E402

FDAT_ENTRIES = os.path.join(A.ROOT, "disc_map", "extracted", "FDAT_T", "entries")
SEQ_ENTRY = 200                      # FDAT.T entry holding the sound-sequence blobs
TALK_FILE_INDEX0 = 3                 # XA file table: entries 3..8 = TALK00..05
XA_LINE_BASE = 0x30                  # X<n> -> FUN_8004DB18(n + 0x30)
N_TALK_LINES = 96
# 20-byte blob header as it appears three times in entry 200 (u32 table offset 0x14 ...)
BLOB_HEADER = bytes.fromhex("14000000 001e0000 32c83200 ffff0000 ff000000")
OPS_RE = re.compile(rb"\x00?((?:[A-Za-z]\d*)+)")
TOKEN_RE = re.compile(rb"([A-Za-z])(\d*)")
PUNCT = {0x8E: ".", 0x8F: ",", 0x90: "?", 0x91: "!", 0x98: ":", 0x9D: "'", 0x8B: "-"}


# ---------------------------------------------------------------- the blob
def decode_text(b):
    """Subtitle bytes in the game's font encoding -> str (unknown codes as <xx>)."""
    out = []
    for c in b:
        if c == 0:
            break
        if 0xAA <= c < 0xAA + 26:
            out.append(chr(ord("A") + c - 0xAA))
        elif 0xC4 <= c < 0xC4 + 26:
            out.append(chr(ord("a") + c - 0xC4))
        elif 0xA0 <= c < 0xAA:
            out.append(chr(ord("0") + c - 0xA0))
        elif c in (0x20, 0x0A):
            out.append(chr(c))
        else:
            out.append(PUNCT.get(c, f"<{c:02x}>"))
    return re.sub(r"[ \t]*\n[ \t]*", " ", "".join(out)).strip()


def parse_tokens(ops):
    """b'M3X40N' -> [('M', 3), ('X', 40), ('N', None)]"""
    return [(m.group(1).decode(), int(m.group(2)) if m.group(2) else None)
            for m in TOKEN_RE.finditer(ops)]


def parse_blob(data, base):
    """Sequences of the blob starting at `base`: {snd: {ops, tokens, text}}."""
    table_off = struct.unpack_from("<I", data, base)[0]
    offs = []
    p = base + table_off
    while p + 2 <= len(data):
        v = struct.unpack_from("<H", data, p)[0]
        if offs and (v <= offs[-1] or v - offs[-1] > 4096):
            break
        if offs and base + offs[0] <= p:
            break
        offs.append(v)
        p += 2
    seqs = {}
    for i, off in enumerate(offs):
        end = base + offs[i + 1] if i + 1 < len(offs) else base + off + 512
        raw = data[base + off:end]
        m = OPS_RE.match(raw)
        ops = m.group(1) if m else b""
        seqs[i] = {"ops": ops.decode(), "tokens": parse_tokens(ops),
                   "text": decode_text(raw[m.end():] if m else raw)}
    return seqs


def find_blobs(data):
    return [m.start() for m in re.finditer(re.escape(BLOB_HEADER), data)]


def line_of(n):
    """TALK line number n (0..95) -> (xa_id, 'TALK0f', channel)."""
    f, c = divmod(n, 16)
    return XA_LINE_BASE + n, f"TALK{f:02d}", c


def line_number(source_file, channel):
    """'GG/BGM/TALK03.XA', 9 -> 57 ; None for anything that is not a TALK file."""
    m = re.search(r"TALK(\d\d)\.XA$", source_file or "")
    return 16 * int(m.group(1)) + int(channel) if m else None


# ------------------------------------------------------------ mission scripts
def mission_sounds(fdat_entries):
    """{mission: {play_sound: [...], call_db18: [...]}} from every 2N+1 FDAT entry
    that carries a chunk-4 script (tools/mission/mission_script.py)."""
    import mission_script as MS
    out = {}
    for p in sorted(glob.glob(os.path.join(fdat_entries, "*.bin"))):
        idx = int(os.path.basename(p)[:3])
        if idx % 2 == 0:
            continue
        try:
            model = MS.parse_stream(open(p, "rb").read())
        except Exception:  # noqa: BLE001 - not a mission stream
            model = None
        if model is None:
            continue
        snds, db18 = [], []
        for s in model.sets:
            for t in s.threads:
                for o in t.ops:
                    if o.opcode == 0x1004:
                        snds.append(o.operands[0])
                    elif o.opcode == 0x1007:
                        db18.append(o.operands[0])
        out[(idx - 1) // 2] = {"play_sound": snds, "call_db18": db18}
    return out


# ---------------------------------------------------------------- the scan
def scan(fdat_entries=FDAT_ENTRIES):
    seq_path = os.path.join(fdat_entries, f"{SEQ_ENTRY:03d}.bin")
    if not os.path.exists(seq_path):
        raise FileNotFoundError(f"{seq_path}: run tools/extract/extract_t.py --file GG/COM/FDAT.T --dump-entries")
    data = open(seq_path, "rb").read()
    bases = find_blobs(data)
    if not bases:
        raise ValueError(f"{seq_path}: no sound-sequence blob header found")
    sequences = {}
    for bi, base in enumerate(bases):
        for snd, s in parse_blob(data, base).items():
            rec = sequences.setdefault(snd, {"ops": s["ops"], "xa_lines": [], "music": [],
                                             "text": s["text"], "blobs": []})
            rec["blobs"].append(bi)
            for letter, n in s["tokens"]:
                if letter in "Xx" and n is not None and n not in rec["xa_lines"]:
                    rec["xa_lines"].append(n)
                if letter in "Vv" and n is not None and n not in rec["music"]:
                    rec["music"].append(n)
            if not rec["text"] and s["text"]:
                rec["text"] = s["text"]
    missions = mission_sounds(fdat_entries)
    for snd, rec in sequences.items():
        rec["missions"] = sorted({m for m, d in missions.items() if snd in d["play_sound"]})
    lines = {}
    for n in range(N_TALK_LINES):
        xa_id, f, c = line_of(n)
        seqs = sorted(snd for snd, rec in sequences.items() if n in rec["xa_lines"])
        ms = sorted({m for snd in seqs for m in sequences[snd]["missions"]})
        texts = [sequences[snd]["text"] for snd in seqs if sequences[snd]["text"]]
        lines[n] = {"xa_id": f"0x{xa_id:02X}", "file": f, "channel": c, "sequences": seqs,
                    "missions": ms, "text": texts[0] if texts else ""}
    return {"source": os.path.relpath(seq_path, A.ROOT).replace("\\", "/"),
            "blobs": [{"offset": b, "n_sequences": len(parse_blob(data, b))} for b in bases],
            "lines": {str(k): v for k, v in lines.items()},
            "sequences": {str(k): v for k, v in sorted(sequences.items())},
            "missions": {str(k): v for k, v in sorted(missions.items())}}


def status_of(line_rec):
    """'referenced' / 'unreferenced' / 'unchecked' + a one-line detail for a lines[n]
    record. Conservative: a line is unreferenced only when NO sound sequence
    contains its `X n` — most English lines sit in sequences that no chunk-4
    script calls (the primary objective VM / game code must trigger those), so
    "sequence exists, no script" still counts as referenced."""
    if line_rec is None:
        return "unchecked", "no scan data"
    seqs = ",".join(map(str, line_rec["sequences"]))
    if line_rec["missions"]:
        return "referenced", (f"sequence {seqs} played by mission script "
                              f"{','.join(map(str, line_rec['missions']))}")
    if line_rec["sequences"]:
        return "referenced", (f"sequence {seqs} plays it; no chunk-4 script calls that sequence "
                              f"(other triggers not scanned)")
    return "unreferenced", "no sound sequence plays this line"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--fdat-entries", default=FDAT_ENTRIES)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--print", action="store_true", help="one line per TALK line")
    args = ap.parse_args()
    res = scan(args.fdat_entries)
    p = os.path.join(args.out, "xa_line_refs.json")
    A.save_json(p, res)
    lines = res["lines"]
    by = {}
    for n, rec in lines.items():
        st, _ = status_of(rec)
        by[st] = by.get(st, 0) + 1
    A.log(f"[xa_refs] {len(res['blobs'])} sequence blobs, {len(res['sequences'])} sequences, "
          f"{len(res['missions'])} mission scripts; TALK lines {by}")
    if args.print:
        for n, rec in lines.items():
            st, why = status_of(rec)
            A.log(f"  {rec['file']} ch{rec['channel']:<2} line {int(n):2d} {rec['xa_id']} {st:<12} {why}  "
                  f"{rec['text'][:50]!r}")
    A.log(f"[xa_refs] -> {p}")


if __name__ == "__main__":
    main()
