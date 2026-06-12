#!/usr/bin/env python3
"""object_stats.py - decode AC1 mission object/MT per-instance stats.

Target: SLUS-01323 (v1.1).  Companion to docs/OBJECT_STATS.md.  Pristine bin only.

KEY RESULT (see docs/OBJECT_STATS.md): AC1 has NO global "stats table keyed by
type id" (no `base + type*stride` HP/damage/name array in the in-mission overlay
FDAT entry 202).  Instead:
  * The displayable PER-INSTANCE numbers (HP-/range-/area-like) are carried in the
    chunk-12 spawn record itself, halfwords hw8..hw19, which the spawner
    FUN_80073B74 copies verbatim into the runtime instance template 0x8019FAB8.
  * The TYPE id (hw7) selects a model/behavior resource block (resource-slot table
    0x801A5F34), not a numeric stat row.

So this tool reports, for a given type id, every spawn instance of that type in a
mission (with its hw8..hw19 decoded under the current field hypotheses), and -- if
you pass an explicit 20-halfword record -- decodes that single record.  It does NOT
invent a type stat row that does not exist in the ROM.

Reuses tools/extract/extract_t.py (container) and tools/mission/mission_parse.py (chunk walk +
record layout) by import; it does NOT modify them.

Usage:
  # all instances of type 5 across mission 1, decoded:
  tools/mission/object_stats.py --fdat disc_map/extracted/FDAT_T/entries --mission 1 --type 5
  # every type used in a mission, with instance stats:
  tools/mission/object_stats.py --fdat disc_map/extracted/FDAT_T/entries --mission 1
  # a single stream file:
  tools/mission/object_stats.py --stream .../003.bin --type 99
  # decode one explicit record (20 comma-separated int16 halfwords):
  tools/mission/object_stats.py --record 28015,-625,5003,0,0,1024,0,77,0,0,0,2050,313,0,0,0,0,0,0,0
"""
import argparse
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

# Reuse the canonical chunk walker + record layout (do NOT duplicate it).
import mission_parse as mp  # noqa: E402

# Per-instance field schema (hw index -> (label, status)).  See docs/OBJECT_STATS.md
# table.  CONFIRMED entries are byte/disasm-verified; the rest are HYPOTHESIS and
# are flagged as such so a viewer can render them honestly.
FIELDS = [
    (0,  "X",            "CONFIRMED"),
    (1,  "Y",            "CONFIRMED"),
    (2,  "Z",            "CONFIRMED"),
    (3,  "geom_block",   "CONFIRMED"),   # -1 = none
    (4,  "hw4",          "HYP"),
    (5,  "rotation",     "CONFIRMED"),   # PSX angle units (4096 = full turn)
    (6,  "hw6",          "HYP"),
    (7,  "TYPE_id",      "CONFIRMED"),
    (8,  "paramA(AI?)",  "HYP"),
    (9,  "paramB",       "HYP"),
    (10, "hw10(link?)",  "HYP"),
    (11, "BIG(HP?)",     "HYP"),         # candidate HP/armor/detection-range
    (12, "sub(range?)",  "HYP"),
    (13, "hw13",         "HYP"),
    (14, "hw14",         "HYP"),
    (15, "area_id?",     "HYP"),
    (16, "hw16",         "HYP"),
    (17, "hw17",         "HYP"),
    (18, "hw18",         "HYP"),
    (19, "hw19",         "HYP"),
]

# Halfwords worth surfacing as "stats" (skip pos/rot/type/block which are not stats)
STAT_HWS = [8, 9, 10, 11, 12, 13, 14, 15]


def load_spawns(buf):
    """Return list of (idx, hw[20]) for every non-empty spawn record."""
    spawn = next((c for c in mp.walk_chunks(buf)
                  if c[0] == mp.SPAWN_CHUNK and not c[3]), None)
    if spawn is None:
        return []
    _, off, ln, _ = spawn
    pay = buf[off + 4: off + 4 + ln]
    out = []
    for i in range(ln // mp.SPAWN_REC_LEN):
        hw = struct.unpack_from("<20h", pay, i * mp.SPAWN_REC_LEN)
        empty = (hw[3] == -1 and hw[0] == 0 and hw[1] == 0
                 and hw[2] == 0 and hw[7] == 0)
        if not empty:
            out.append((i, hw))
    return out


def decode_record(hw):
    """Yield (hw_index, label, value, status) for a 20-halfword record."""
    for k, label, status in FIELDS:
        yield k, label, hw[k], status


def print_record(hw, prefix=""):
    print(f"{prefix}type={hw[7]}  pos=({hw[0]},{hw[1]},{hw[2]})  "
          f"rot={hw[5]}  geom_block={hw[3]}")
    parts = []
    for k in STAT_HWS:
        label = FIELDS[k][1]
        parts.append(f"hw{k}({label})={hw[k]}")
    print(f"{prefix}  stats[HYP]: " + "  ".join(parts))


def print_type(spawns, type_id):
    rows = [(i, hw) for (i, hw) in spawns if hw[7] == type_id]
    if not rows:
        print(f"type {type_id}: no instances in this mission")
        return
    print(f"type {type_id}: {len(rows)} instance(s)")
    print("  NOTE: per docs/OBJECT_STATS.md, AC1 has no per-type stat row; the")
    print("  numbers below are PER-INSTANCE (spawn hw8..hw19), HYPOTHESIS labels.")
    # Show whether the stat fields are constant across instances of this type
    # (constant -> likely a shared group param; varying -> likely a real stat).
    for i, hw in rows:
        print_record(hw, prefix=f"  rec{i:3d}  ")
    for k in STAT_HWS:
        vals = sorted({hw[k] for _, hw in rows})
        if len(vals) == 1:
            tag = "CONSTANT across this type's instances (group/shared param?)"
        else:
            tag = f"VARIES {vals} (per-instance -> better stat candidate)"
        print(f"    hw{k:<2d} {FIELDS[k][1]:<12s}: {tag}")


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--fdat", help="dir of extracted FDAT entries (NNN.bin)")
    ap.add_argument("--mission", type=int, help="mission number (uses entry 2N+1)")
    ap.add_argument("--stream", help="path to a single chunk-stream entry (NNN.bin)")
    ap.add_argument("--type", type=int, help="filter to this object/MT type id (hw7)")
    ap.add_argument("--record", help="decode one explicit 20-halfword record (csv)")
    args = ap.parse_args()

    if args.record:
        hw = [int(x) for x in args.record.split(",")]
        if len(hw) != 20:
            ap.error("--record needs exactly 20 comma-separated int16 halfwords")
        print("=== explicit record ===")
        print_record(hw, prefix="  ")
        print()
        for k, label, val, status in decode_record(hw):
            print(f"  hw{k:<2d} {label:<14s} = {val:<8d} [{status}]")
        return

    if args.stream:
        path = args.stream
    elif args.fdat is not None and args.mission is not None:
        path = os.path.join(args.fdat, f"{2 * args.mission + 1:03d}.bin")
    else:
        ap.error("give --record CSV, or --stream PATH, or --fdat DIR --mission N")

    with open(path, "rb") as f:
        buf = f.read()
    print(f"=== {path} ===")
    spawns = load_spawns(buf)
    if not spawns:
        print("no spawn chunk (index 12) found")
        return

    if args.type is not None:
        print_type(spawns, args.type)
    else:
        types = sorted({hw[7] for _, hw in spawns})
        print(f"types present ({len(types)}): {types}")
        print()
        for t in types:
            print_type(spawns, t)
            print()


if __name__ == "__main__":
    main()
