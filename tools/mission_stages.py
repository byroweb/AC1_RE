#!/usr/bin/env python3
"""mission_stages.py - resolve a mission's PA stage file(s) and its spawn
block-index -> PA/chunk geometry, for AC1 SLUS-01323 (v1.1).

Two linked unknowns are answered here (see docs/MISSION_STAGES.md):

(1) MISSION -> PA STAGE FILE
    The in-mission overlay's PA loader FUN_8004F1A8 builds the stage path from a
    single GameState byte DAT_8004121B:
        phase n  = byte // 20     (P0/P1/P2/P3 directory)
        PA index = byte           (formatted as PA%02d using tens=byte//10,
                                    ones=byte%10)  ->  "P{n}\\PA{byte:02d}.T"
    i.e. the PA FILE NUMBER IS THE STAGE BYTE ITSELF (CONFIRMED by disassembly of
    the 0xCCCCCCCD divide-by-10/20 magic at 0x8004F1A8).
    Eight optional SUB-RESOURCE bytes DAT_8004121D..0x1224 each select an extra
    *.T entry index inside the SAME PA file (file slot 0) as `byte + addend`
    (255/0xFF = skip).  CONFIRMED addends below.

    *** The per-mission VALUE of DAT_8004121B is NOT stored statically in either
        the overlay (FDAT entry 202) or the base EXE (SLUS_013.23); it is written
        into the GameState block at runtime by the menu/mission-launch path.
        So this tool can DECODE a stage byte into its PA file(s), and it accepts
        the stage byte either from --stage N or from a user-supplied/ground-truth
        map (--stagemap), but it cannot derive the mission->byte map from the two
        binaries alone.  Ground-truth the byte in DuckStation (read 0x8004121B at
        mission start) and record it in the map.  See docs/MISSION_STAGES.md. ***

(2) SPAWN BLOCK-INDEX -> GEOMETRY  (CONFIRMED, verified missions 0-3)
    chunk-12 spawn record halfword hw3 (source byte 0x06; -1 = none) is the
    block index a1[+0x0A] used by FUN_80078B14 to index the 44-byte record table
    at 0x8019F538 (blkrec = 0x8019F538 + hw3*44; geometry ptr at blkrec[+0x28]):
        hw3 == -1 -> no bound geometry (object rendered by a type/other path)
        hw3 == 0  -> record[0] = the PA file's ENTRY 1 placement directory
                     (registered by PA loader FUN_8004F1A8 via FUN_80073AD8)
        hw3 == k>0-> record[k] = chunk-11 sub-record (k-1), a per-mission geometry
                     block carried INSIDE the mission chunk stream (entry 2N+1),
                     registered by chunk-11 handler FUN_800739AC into
                     0x8019F564 + (k-1)*44 (= 0x8019F538 + k*44).
    Verified: chunk-11 record count == max hw3 for missions 0,1,2,3.

Usage:
  tools/mission_stages.py --fdat DIR --mission N [--stage BYTE] [--stagemap FILE]
  tools/mission_stages.py --decode-stage BYTE         # just print PA file(s)
  tools/mission_stages.py --stream 2N+1.bin --stage BYTE
"""
import argparse
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mission_parse import walk_chunks, SPAWN_CHUNK, SPAWN_REC_LEN  # reuse, do not edit

# ---- (1) PA loader constants (CONFIRMED from FDAT entry-202 disasm) -----------
# FUN_8004F1A8: path "P{byte//20}\PA{byte:02d}.T", PA file number == stage byte.
# Sub-resource bytes 0x121D..0x1224 -> entry index = byte + ADDEND (255 = skip),
# loaded from the SAME PA file (file slot 0).  Addends read straight from disasm.
SUBRES = [
    ("0x8004121D", 0x02),
    ("0x8004121E", 0x0A),
    ("0x8004121F", 0x2C),
    ("0x80041220", 0x58),
    ("0x80041221", 0x3C),
    ("0x80041222", 0x46),
    ("0x80041223", 0x6C),
    ("0x80041224", 0x8E),
]

CHUNK11 = 11   # per-mission geometry-block chunk -> record table 0x8019F538[k>0]


def pa_path(stage_byte):
    """Return ('P{n}', 'PA{nn}.T', 'GG/P{n}/PA{nn}.T') for a stage byte."""
    n = stage_byte // 20
    nn = stage_byte                      # PA number printed == byte (PA%02d-ish)
    fname = f"PA{nn:02d}.T"
    pdir = f"P{n}"
    return pdir, fname, f"GG/{pdir}/{fname}"


def decode_stage(stage_byte, subres_bytes=None):
    pdir, fname, full = pa_path(stage_byte)
    print(f"stage byte 0x{stage_byte:02x} ({stage_byte}) -> {full}  "
          f"(phase {pdir}, PA #{stage_byte})")
    if subres_bytes is not None:
        print("  sub-resource PA entries (same file, slot 0):")
        for i, (addr, addend) in enumerate(SUBRES):
            b = subres_bytes[i] if i < len(subres_bytes) else 0xFF
            if b == 0xFF:
                print(f"    [{addr}] = 0xFF  (none)")
            else:
                print(f"    [{addr}] = {b:3d} -> PA .T entry {b + addend} "
                      f"(byte + 0x{addend:02x})")


# ---- chunk-11 record table (CONFIRMED handler FUN_800739AC) -------------------
def chunk11_records(buf):
    """Return list of (subrec_len) for chunk-11; index i -> 0x8019F538 record i+1.
    Handler: count = u32[0]; per record sublen = u32; advance sublen+4."""
    ch = next((c for c in walk_chunks(buf) if c[0] == CHUNK11 and not c[3]), None)
    if ch is None:
        return []
    _, off, ln, _ = ch
    pay = buf[off + 4: off + 4 + ln]
    if len(pay) < 4:
        return []
    count = struct.unpack_from("<I", pay, 0)[0]
    out = []
    p = 4
    for _ in range(count):
        if p + 4 > len(pay):
            break
        sublen = struct.unpack_from("<I", pay, p)[0]
        out.append(sublen)
        p += sublen + 4
    return out


def resolve_block(hw3, n_chunk11, stage_full):
    """Map a spawn block index hw3 -> human description of bound geometry."""
    if hw3 == -1:
        return "(none: hw3 == -1, no bound geometry)"
    if hw3 == 0:
        return f"record[0] = PA entry 1 placement directory of {stage_full}"
    if 1 <= hw3 <= n_chunk11:
        return (f"record[{hw3}] = chunk-11 sub-record {hw3 - 1} "
                f"(per-mission geometry, embedded in mission stream)")
    return (f"record[{hw3}] = OUT OF RANGE "
            f"(only {n_chunk11} chunk-11 records present -> uninitialised)")


def dump_mission(buf, stage_byte=None, subres_bytes=None, stage_full=None):
    recs11 = chunk11_records(buf)
    n11 = len(recs11)
    if stage_byte is not None:
        decode_stage(stage_byte, subres_bytes)
    print(f"\nchunk-11 geometry records: {n11} "
          f"(-> 0x8019F538 records 1..{n11}; sub-lens {recs11})")
    # spawn table
    spawn = next((c for c in walk_chunks(buf)
                  if c[0] == SPAWN_CHUNK and not c[3]), None)
    if spawn is None:
        print("no spawn chunk", file=sys.stderr)
        return
    _, off, ln, _ = spawn
    pay = buf[off + 4: off + 4 + ln]
    print(f"\nspawn block-index resolution ({ln // SPAWN_REC_LEN} slots):")
    print(" idx  type  hw3(blk)  resolves to")
    for i in range(ln // SPAWN_REC_LEN):
        hw = struct.unpack_from("<20h", pay, i * SPAWN_REC_LEN)
        empty = (hw[3] == -1 and hw[0] == 0 and hw[1] == 0
                 and hw[2] == 0 and hw[7] == 0)
        if empty:
            continue
        desc = resolve_block(hw[3], n11,
                             stage_full or "(stage PA file - pass --stage)")
        print(f" {i:3d}  {hw[7]:4d}  {hw[3]:8d}  {desc}")


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--fdat", help="dir of extracted FDAT entries (NNN.bin)")
    ap.add_argument("--mission", type=int, help="mission number (uses entry 2N+1)")
    ap.add_argument("--stream", help="explicit chunk-stream entry (NNN.bin)")
    ap.add_argument("--stage", type=lambda s: int(s, 0),
                    help="stage byte DAT_8004121B (PA file number) if known")
    ap.add_argument("--subres", help="comma list of 8 sub-resource bytes 0x121D..0x1224")
    ap.add_argument("--stagemap", help="JSON {mission:int -> stage_byte:int} (ground-truth)")
    ap.add_argument("--decode-stage", type=lambda s: int(s, 0),
                    help="just decode a stage byte to its PA file(s) and exit")
    args = ap.parse_args()

    if args.decode_stage is not None:
        sub = None
        if args.subres:
            sub = [int(x, 0) for x in args.subres.split(",")]
        decode_stage(args.decode_stage, sub)
        return

    if args.stream:
        path = args.stream
    elif args.fdat is not None and args.mission is not None:
        path = os.path.join(args.fdat, f"{2 * args.mission + 1:03d}.bin")
    else:
        ap.error("give --stream PATH  or  --fdat DIR --mission N  (or --decode-stage)")

    stage = args.stage
    if stage is None and args.stagemap and args.mission is not None:
        m = json.load(open(args.stagemap))
        stage = m.get(str(args.mission), m.get(args.mission))
    sub = None
    if args.subres:
        sub = [int(x, 0) for x in args.subres.split(",")]

    stage_full = None
    if stage is not None:
        stage_full = pa_path(stage)[2]

    with open(path, "rb") as f:
        buf = f.read()
    print(f"=== {path} ===")
    if args.mission is not None:
        print(f"mission {args.mission}  (FDAT entries {2*args.mission} obj / "
              f"{2*args.mission+1} stream)")
    dump_mission(buf, stage_byte=stage, subres_bytes=sub, stage_full=stage_full)


if __name__ == "__main__":
    main()
