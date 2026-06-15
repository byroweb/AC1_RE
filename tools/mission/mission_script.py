#!/usr/bin/env python3
"""mission_script.py - (dis)assembler for the AC1 SECONDARY mission VM (chunk 4).

Target: SLUS-01323 (v1.1).  Companion to scratch/re/secondary_vm.md (full RE) and
mission_parse.py (chunk-stream conventions).

The SECONDARY VM is a per-frame actor-thread interpreter (tick @0x8008B380,
dispatch @0x8008B42C, loader @0x8008C178) that drives timed scripted actors:
spawn groups, cutscene-like enemy entrances, moving/lerping objects, timed sound/
marker triggers and mission result-flag sets.  Its bytecode = CHUNK INDEX 4 of a
mission's 2N+1 chunk stream (see mission_parse.walk_chunks).

On-disc blob format (secondary_vm.md sec.3, byte-verified):

  blob:
    u16  set_offset[NSETS]          ; NSETS = set_offset[0] / 2
    ...  set headers + thread bytecode ...

  set header (at blob + set_offset[s]):
    u16  flags                      ; +0
    u16  thread_count               ; +2
    u16  extra                      ; +4
    u16  thread_off[thread_count]   ; +6  (byte offsets within blob)

  thread (at blob + thread_off[i]):
    u16  thread_hdr                 ; +0  (PC starts at +2; selects scratch size)
    ...  opcodes (sec.5 table) ...
    u16  END (0xFFFF)

Opcode encoding: u16 opcode (LE) + OPLEN(opcode) bytes of LE int16 operands.
Unknown/reserved opcode words (0x000C/0x000D/0x0010/...) are 0-operand silent
no-ops in v1.1; they are preserved verbatim on round-trip.

Usage:
  tools/mission/mission_script.py --fdat disc_map/extracted/FDAT_T/entries --mission 0
  tools/mission/mission_script.py --stream disc_map/extracted/FDAT_T/entries/001.bin
  tools/mission/mission_script.py --selftest          # round-trip every 2N+1 entry
  tools/mission/mission_script.py --selftest --fdat OTHER_DIR
"""
import argparse
import os
import struct
import sys
from dataclasses import dataclass, field
from typing import List, Optional

# Reuse the chunk-stream walker so the two tools stay in lock-step.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mission_parse import walk_chunks  # noqa: E402

SCRIPT_CHUNK = 4          # index of the secondary-VM blob in the chunk stream


# ---------------------------------------------------------------------------
# Opcode table (single source of truth; copy of secondary_vm.md sec.5)
#
# Each entry: value -> (mnemonic, oplen_rule).
#   oplen_rule is an int = fixed operand bytes AFTER the 2-byte opcode word,
#   or the string "op_f" for the only variable-length opcode (0x000F).
# Operands are decoded/encoded as little-endian int16 halfwords.
# Any value NOT in this table is a 0-operand reserved no-op (v1.1).
# ---------------------------------------------------------------------------
OPS = {
    0x0000: ("copy_pos_lo",   6),
    0x0002: ("copy_pos_hi",   6),
    0x0008: ("spawn_at8",     6),
    0x0009: ("spawn_at9",     6),
    0x000A: ("init_unit",     12),
    0x000B: ("subcall_b320",  0),
    0x000E: ("mark_lerp_e",   14),
    0x000F: ("op_f",          "op_f"),   # u16 cnt, then cnt*4 bytes
    0x1000: ("sel_actor",     2),
    0x1001: ("set_xyz",       6),
    0x1002: ("set_xyz2",      6),
    0x1003: ("spawn_group",   2),
    0x1004: ("play_sound",    2),
    0x1005: ("flag_all",      0),
    0x1006: ("clear_targets", 0),
    0x1007: ("call_db18",     2),
    0x1008: ("op_1008",       6),
    0x1009: ("spawn_marker",  6),
    0x100A: ("set_result",    2),
    0x100B: ("set_obj_word",  4),
    0x100C: ("despawn_group", 2),
    0xFFF0: ("set_loop",      2),
    0xFFFF: ("END",           0),
}
NAME_TO_OP = {name: op for op, (name, _) in OPS.items()}
NOOP_NAME = "noop"            # synthetic mnemonic for reserved/unknown words


# ---------------------------------------------------------------------------
# Structured model
# ---------------------------------------------------------------------------
@dataclass
class Op:
    """One decoded instruction."""
    offset: int               # byte offset of the opcode word within the blob
    opcode: int               # raw u16 opcode value
    name: str                 # mnemonic (or NOOP_NAME for reserved words)
    raw: bytes                # the FULL instruction bytes incl. opcode word
    operands: List[int] = field(default_factory=list)  # decoded int16 fields

    @property
    def is_noop(self) -> bool:
        return self.name == NOOP_NAME

    @property
    def is_end(self) -> bool:
        return self.opcode == 0xFFFF


@dataclass
class Thread:
    offset: int               # blob byte offset of the thread (thread_hdr)
    hdr: int                  # thread_hdr (selects scratch size; preserve)
    ops: List[Op] = field(default_factory=list)


@dataclass
class ScriptSet:
    offset: int               # blob byte offset of the set header
    flags: int
    extra: int
    thread_offs: List[int]    # original thread_off[] values (for fidelity)
    threads: List[Thread] = field(default_factory=list)


@dataclass
class Blob:
    raw: bytes                # original chunk-4 payload (round-trip oracle)
    set_offs: List[int]       # set_offset[] table values
    sets: List[ScriptSet] = field(default_factory=list)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def _hw(b: bytes, o: int) -> int:
    return struct.unpack_from("<H", b, o)[0]


def _sh(b: bytes, o: int) -> int:
    return struct.unpack_from("<h", b, o)[0]


def get_script_blob(stream: bytes) -> Optional[bytes]:
    """Return chunk-4 payload bytes from a 2N+1 chunk stream, or None."""
    for idx, off, ln, stop in walk_chunks(stream):
        if stop:
            return None
        if idx == SCRIPT_CHUNK:
            return stream[off + 4: off + 4 + ln]
    return None


def op_total_len(blob: bytes, pc: int) -> int:
    """Total instruction length (opcode word + operands) at blob offset pc."""
    op = _hw(blob, pc)
    if op == 0x000F:                       # op_f: u16 cnt, then cnt*4 bytes
        cnt = _hw(blob, pc + 2)
        return 2 + 2 + cnt * 4
    _, rule = OPS.get(op, (NOOP_NAME, 0))
    return 2 + (rule if isinstance(rule, int) else 0)


def decode_op(blob: bytes, pc: int) -> Op:
    """Decode a single instruction at blob offset pc."""
    op = _hw(blob, pc)
    if op == 0x000F:
        cnt = _hw(blob, pc + 2)
        total = 2 + 2 + cnt * 4
        raw = blob[pc:pc + total]
        # operands: the count, then the cnt pairs flattened as int16s
        operands = [cnt]
        for i in range(cnt):
            operands.append(_sh(blob, pc + 4 + i * 4))
            operands.append(_sh(blob, pc + 6 + i * 4))
        return Op(pc, op, "op_f", raw, operands)
    name, rule = OPS.get(op, (NOOP_NAME, 0))
    oplen = rule if isinstance(rule, int) else 0
    total = 2 + oplen
    raw = blob[pc:pc + total]
    operands = [_sh(blob, pc + 2 + 2 * i) for i in range(oplen // 2)]
    return Op(pc, op, name, raw, operands)


# ---------------------------------------------------------------------------
# parse: bytes -> Blob model
# ---------------------------------------------------------------------------
def parse(blob: bytes) -> Blob:
    """Parse a chunk-4 payload into a structured Blob model."""
    nsets = _hw(blob, 0) // 2
    set_offs = [_hw(blob, 2 * s) for s in range(nsets)]
    model = Blob(raw=blob, set_offs=set_offs)
    for so in set_offs:
        # A set_offset of 0 (or one that overlaps the set_offset table itself)
        # is a sentinel/placeholder slot: the loader would only deref it if
        # cmd 2 requested that index, but no shipped mission does.  Record an
        # empty ScriptSet so the offset value round-trips, but don't walk it.
        # (Seen in 019.bin: set_offsets=[8,36,0,60] -> index 2 is a sentinel.)
        if so < nsets * 2 or so + 6 > len(blob):
            model.sets.append(ScriptSet(offset=so, flags=0, extra=0,
                                        thread_offs=[]))
            continue
        flags = _hw(blob, so)
        count = _hw(blob, so + 2)
        extra = _hw(blob, so + 4)
        thread_offs = [_hw(blob, so + 6 + 2 * i) for i in range(count)]
        sset = ScriptSet(offset=so, flags=flags, extra=extra,
                         thread_offs=thread_offs)
        for toff in thread_offs:
            hdr = _hw(blob, toff)
            thread = Thread(offset=toff, hdr=hdr)
            pc = toff + 2                      # PC starts after thread_hdr
            guard = 0
            while pc < len(blob) and guard < 4096:
                o = decode_op(blob, pc)
                thread.ops.append(o)
                pc += len(o.raw)
                guard += 1
                if o.is_end:
                    break
            sset.threads.append(thread)
        model.sets.append(sset)
    return model


def parse_stream(stream: bytes) -> Optional[Blob]:
    """Convenience: extract chunk 4 from a full 2N+1 stream and parse it."""
    blob = get_script_blob(stream)
    if blob is None:
        return None
    return parse(blob)


# ---------------------------------------------------------------------------
# assemble: Blob model -> bytes (must reproduce chunk-4 payload exactly)
# ---------------------------------------------------------------------------
def encode_op(o: Op) -> bytes:
    """Encode a single Op back to bytes.

    Re-derives the bytes from opcode + operands so that an *edited* model
    assembles correctly.  (o.raw is the round-trip oracle but is NOT used here,
    so authored edits actually take effect.)
    """
    out = struct.pack("<H", o.opcode)
    if o.opcode == 0x000F:
        cnt = o.operands[0]
        out += struct.pack("<H", cnt)
        # operands[1:] are the flattened cnt*2 halfwords
        for v in o.operands[1:1 + cnt * 2]:
            out += struct.pack("<h", v)
        return out
    for v in o.operands:
        out += struct.pack("<h", v)
    return out


def assemble(model: Blob) -> bytes:
    """Rebuild the chunk-4 payload from a Blob model.

    Layout strategy mirrors the original on-disc packing exactly:
      [set_offset table][set0 header+threads][set1 ...]...
    Set/thread offsets are taken from the model (preserved on parse, or supply
    your own when authoring).  Sets and threads are emitted in offset order.
    """
    # 1. Build each thread body keyed by its declared offset.
    thread_bytes = {}                       # thread.offset -> bytes
    for sset in model.sets:
        for thread in sset.threads:
            body = struct.pack("<H", thread.hdr)
            for o in thread.ops:
                body += encode_op(o)
            thread_bytes[thread.offset] = body

    # 2. Build each set header keyed by its declared offset.  Skip sentinel
    #    sets (offset overlapping the set_offset table) so we don't clobber it.
    nsets = len(model.set_offs)
    set_bytes = {}                          # set.offset -> header bytes
    for sset in model.sets:
        if sset.offset < nsets * 2 or sset.offset + 6 > len(model.raw):
            continue                        # sentinel slot; nothing to emit
        hdr = struct.pack("<HHH", sset.flags, len(sset.threads), sset.extra)
        for toff in sset.thread_offs:
            hdr += struct.pack("<H", toff)
        set_bytes[sset.offset] = hdr

    # 3. Assemble flat buffer using the recorded absolute offsets so the byte
    #    layout matches the original exactly.  We write the set_offset table,
    #    then drop every set header and thread body at its declared offset.
    pieces = {}                             # offset -> bytes
    pieces[0] = b"".join(struct.pack("<H", v) for v in model.set_offs)
    for off, b in set_bytes.items():
        pieces[off] = b
    for off, b in thread_bytes.items():
        pieces[off] = b

    total = len(model.raw)
    buf = bytearray(model.raw if total else b"")   # start from original size
    # Overwrite each region (handles any inter-region padding faithfully).
    for off in sorted(pieces):
        b = pieces[off]
        end = off + len(b)
        if end > len(buf):
            buf.extend(b"\x00" * (end - len(buf)))
        buf[off:end] = b
    return bytes(buf)


# ---------------------------------------------------------------------------
# Disassembly (pretty-print)
# ---------------------------------------------------------------------------
def _fmt_operands(o: Op) -> str:
    if o.is_noop:
        return ""
    if o.name == "op_f":
        cnt = o.operands[0]
        pairs = o.operands[1:1 + cnt * 2]
        items = ", ".join(f"({pairs[i]},{pairs[i+1]})"
                          for i in range(0, len(pairs), 2))
        return f"cnt={cnt} [{items}]"
    return ", ".join(str(v) for v in o.operands)


def disassemble(model: Blob) -> str:
    """Return a readable per-set/per-thread listing with file offsets."""
    lines = []
    lines.append(f"chunk4 blob: {len(model.raw)} bytes  "
                 f"sets={len(model.sets)}  set_offsets={model.set_offs}")
    for si, sset in enumerate(model.sets):
        lines.append(f"  set{si} @{sset.offset}  flags={sset.flags}  "
                     f"extra={sset.extra}  threads={sset.thread_offs}")
        for thread in sset.threads:
            lines.append(f"   thread @{thread.offset} "
                         f"(hdr=0x{thread.hdr:04x}):")
            for o in thread.ops:
                tag = " ; reserved-noop" if o.is_noop else ""
                ops = _fmt_operands(o)
                ops = f"  {ops}" if ops else ""
                lines.append(f"     [{o.offset:4d}] 0x{o.opcode:04x} "
                             f"{o.name:14s}{ops}{tag}")
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Self-test (round-trip gate)
# ---------------------------------------------------------------------------
def selftest(fdat_dir: str) -> int:
    """assemble(parse(chunk4)) == chunk4 for every 2N+1 entry. Returns exit code."""
    entries = sorted(f for f in os.listdir(fdat_dir) if f.endswith(".bin"))
    odd = [f for f in entries if int(f[:-4]) % 2 == 1]
    passed = total = 0
    failures = []
    for fn in odd:
        path = os.path.join(fdat_dir, fn)
        with open(path, "rb") as f:
            stream = f.read()
        blob = get_script_blob(stream)
        if blob is None or len(blob) < 2:
            continue                          # no script chunk in this entry
        total += 1
        try:
            model = parse(blob)
            rebuilt = assemble(model)
            if rebuilt == blob:
                passed += 1
            else:
                # find first differing byte for the report
                diff = next((i for i in range(min(len(rebuilt), len(blob)))
                             if rebuilt[i] != blob[i]), None)
                failures.append((fn, f"mismatch @byte {diff} "
                                 f"(len {len(rebuilt)} vs {len(blob)})"))
        except Exception as e:               # noqa: BLE001
            failures.append((fn, f"EXC {type(e).__name__}: {e}"))
    print(f"SELFTEST round-trip: {passed}/{total} 2N+1 entries with chunk4 OK")
    for fn, why in failures:
        print(f"  FAIL {fn}: {why}")
    return 0 if passed == total and total > 0 else 1


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--fdat", help="dir of extracted FDAT entries (NNN.bin)")
    ap.add_argument("--mission", type=int, help="mission number (uses entry 2N+1)")
    ap.add_argument("--stream", help="path to a single chunk-stream entry (NNN.bin)")
    ap.add_argument("--selftest", action="store_true",
                    help="round-trip every 2N+1 entry in --fdat (authoring gate)")
    args = ap.parse_args()

    default_fdat = "disc_map/extracted/FDAT_T/entries"

    if args.selftest:
        fdat = args.fdat or default_fdat
        sys.exit(selftest(fdat))

    if args.stream:
        path = args.stream
    elif args.fdat is not None and args.mission is not None:
        path = os.path.join(args.fdat, f"{2 * args.mission + 1:03d}.bin")
    else:
        ap.error("give --stream PATH | --fdat DIR --mission N | --selftest")

    with open(path, "rb") as f:
        stream = f.read()
    print(f"=== {path} ===")
    model = parse_stream(stream)
    if model is None:
        print("no script chunk (index 4) found in this stream", file=sys.stderr)
        sys.exit(1)
    print(disassemble(model))


if __name__ == "__main__":
    main()
