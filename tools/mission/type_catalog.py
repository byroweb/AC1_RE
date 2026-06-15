#!/usr/bin/env python3
"""type_catalog.py - build a master OBJECT-TYPE catalog across all AC1 missions.

Target: SLUS-01323 (v1.1).  Phase 5 of the authoring plan
("Type -> behavior binding").  Companion to docs/MISSION_SYSTEM.md,
docs/OBJECT_STATS.md and scratch/re/objective_object.md.  Pristine bin only.

A mission N is described by a PAIR of FDAT.T entries (file id 2):
  * entry  (2*N)    = the "objective object" image, ALWAYS loaded to fixed addr
                      0x801C4B40 (no relocation; the u32 pointers stored in the
                      file are absolute 0x801Cxxxx, so file_off = ptr - 0x801C4B40).
                      It begins with a method/pointer block; at struct offset
                      +0x20 there is a per-TYPE INIT (ctor) table:
                          INIT[t] = *(0x801C4B40 + 0x20 + t*4)
                      entity-spawn @0x80078D50 calls INIT[type] where
                      type = entity +0x0E (a small per-mission LOGICAL type id,
                      observed 1..6; slot 0 points at the no-op stub).  Each INIT
                      installs the per-entity vtable +0x50..+0x5C incl. +0x58 THINK.
                      (See scratch/re/objective_object.md sections 1-2.)
  * entry  (2*N+1)  = the mission CHUNK STREAM.  Chunk 12 = the 256-record x
                      40-byte spawn / instance table.  Per record (see
                      docs/OBJECT_STATS.md, scratch/re/spawn_marshal.md):
                        hw3  = LOGICAL DISPATCH TYPE (= geom block idx = entity
                               +0x0E; the INIT-table index; small, 1..6/-1)
                        hw7  = RESOURCE / CLASS id (global model selector, 0..388)
                        hw11 = HP (-> instance +0x160/162/164)

IMPORTANT NAMESPACE NOTE (resolved by scratch/re/spawn_marshal.md):
  Two distinct ids.  The objective-object +0x20 INIT table keys off the LOGICAL
  dispatch type = chunk-12 hw3 (NOT hw7).  hw7 is the wide resource/class id
  (flag-table key, never the dispatch).  So the meaningful per-mission join is
  (logical type hw3) <-> INIT[hw3]; this tool does that join, and ALSO reports the
  hw7 resource catalog and hw11 HP values as separate columns.

Reuses tools/mission/mission_parse.py (chunk walk + spawn record layout) by import.

Outputs (written under disc_map/ and scratch/re/):
  * disc_map/type_catalog.csv        - one row per global hw7 TYPE id (aggregated)
  * disc_map/type_catalog_long.csv   - long form: (mission, namespace, type, count, ptr)
  * scratch/re/type_catalog.md       - readable markdown summary

Usage:
  tools/mission/type_catalog.py                       # write all outputs
  tools/mission/type_catalog.py --fdat DIR            # custom entries dir
  tools/mission/type_catalog.py --mission 1           # dump one mission to stdout
"""
import argparse
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import mission_parse as mp  # noqa: E402  (canonical chunk walker + record layout)

REPO = os.path.dirname(os.path.dirname(HERE))
DEFAULT_FDAT = os.path.join(REPO, "disc_map", "extracted", "FDAT_T", "entries")
OUT_CSV = os.path.join(REPO, "disc_map", "type_catalog.csv")
OUT_LONG = os.path.join(REPO, "disc_map", "type_catalog_long.csv")
OUT_MD = os.path.join(REPO, "scratch", "re", "type_catalog.md")

# Objective-object load base + INIT table window (scratch/re/objective_object.md).
OBJOBJ_BASE = 0x801C4B40
INIT_OFF = 0x20            # +0x20 + type*4
INIT_END = 0x60           # METHOD_B table begins at +0x60 -> stop before it
MAX_MISSION = 95          # entries observed up to mission 94

# Display names: MIS.T holds only mission briefing prose, NOT a per-hw7-type name
# table; no global type display-name array exists in the ROM (docs/OBJECT_STATS.md).
# Column kept for completeness but always blank.  See scratch/re/type_catalog_notes.md.
TYPE_NAMES = {}


def stream_path(fdat, n):
    return os.path.join(fdat, f"{2 * n + 1:03d}.bin")


def objobj_path(fdat, n):
    return os.path.join(fdat, f"{2 * n:03d}.bin")


def load_spawn_types(buf):
    """Parse chunk-12. Return dict with three views (empty records skipped):
      resource  {hw7: count}          - global model/class catalog
      logical   {hw3: count}          - dispatch type (INIT-table index)
      hp        {hw3: sorted [hw11]}  - HP values seen per logical type
      res_by_logical {hw3: sorted [hw7]} - which resources use each logical type
    """
    spawn = next((c for c in mp.walk_chunks(buf)
                  if c[0] == mp.SPAWN_CHUNK and not c[3]), None)
    if spawn is None:
        return {"resource": {}, "logical": {}, "hp": {}, "res_by_logical": {}}
    _, off, ln, _ = spawn
    pay = buf[off + 4: off + 4 + ln]
    resource, logical, hp, rbl = {}, {}, {}, {}
    for i in range(ln // mp.SPAWN_REC_LEN):
        hw = struct.unpack_from("<20h", pay, i * mp.SPAWN_REC_LEN)
        empty = (hw[3] == -1 and hw[0] == 0 and hw[1] == 0
                 and hw[2] == 0 and hw[7] == 0)
        if empty:
            continue
        resource[hw[7]] = resource.get(hw[7], 0) + 1
        logical[hw[3]] = logical.get(hw[3], 0) + 1
        hp.setdefault(hw[3], set()).add(hw[11])
        rbl.setdefault(hw[3], set()).add(hw[7])
    return {"resource": resource, "logical": logical,
            "hp": {k: sorted(v) for k, v in hp.items()},
            "res_by_logical": {k: sorted(v) for k, v in rbl.items()}}


def load_init_table(buf):
    """Return {logical_type: init_ptr} from objective-object +0x20..+0x60.

    Stops at +0x60 (METHOD_B) or the first null / out-of-range pointer slot is
    simply skipped (slot 0 normally points at the no-op stub and is skipped as
    type 0)."""
    out = {}
    t = 0
    off = INIT_OFF
    while off < INIT_END and off + 4 <= len(buf):
        ptr = struct.unpack_from("<I", buf, off)[0]
        fo = ptr - OBJOBJ_BASE
        if ptr != 0 and 0 <= fo < len(buf):
            out[t] = ptr
        t += 1
        off += 4
    return out


def collect(fdat):
    """Walk every mission; return per-mission dict and the master aggregates.

    Returns (missions, agg) where:
      missions[N] = {
        'spawns': {hw7: count}, 'init': {logical_type: ptr},
        'has_stream': bool, 'has_objobj': bool, 'has_spawn_chunk': bool }
      agg per global hw7 type id (see build aggregates below).
    """
    missions = {}
    for n in range(MAX_MISSION):
        sp = stream_path(fdat, n)
        op = objobj_path(fdat, n)
        if not (os.path.exists(sp) or os.path.exists(op)):
            continue
        rec = {"spawns": {}, "logical": {}, "hp": {}, "res_by_logical": {},
               "init": {}, "has_stream": False,
               "has_objobj": False, "has_spawn_chunk": False}
        if os.path.exists(sp):
            rec["has_stream"] = True
            buf = open(sp, "rb").read()
            sv = load_spawn_types(buf)
            rec["spawns"] = sv["resource"]      # hw7 resource catalog (back-compat)
            rec["logical"] = sv["logical"]      # hw3 dispatch type
            rec["hp"] = sv["hp"]
            rec["res_by_logical"] = sv["res_by_logical"]
            rec["has_spawn_chunk"] = bool(
                next((c for c in mp.walk_chunks(buf)
                      if c[0] == mp.SPAWN_CHUNK and not c[3]), None))
        if os.path.exists(op):
            rec["has_objobj"] = True
            rec["init"] = load_init_table(open(op, "rb").read())
        missions[n] = rec
    return missions


def build_type_agg(missions):
    """Aggregate by global hw7 TYPE id across missions."""
    agg = {}
    for n, rec in missions.items():
        for t, c in rec["spawns"].items():
            a = agg.setdefault(t, {"missions": set(), "total": 0, "per_m": {}})
            a["missions"].add(n)
            a["total"] += c
            a["per_m"][n] = c
    return agg


# -------------------------------------------------------------------------- I/O

def write_long_csv(missions, path):
    import csv
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["mission", "namespace", "type", "count", "init_ptr",
                    "hp_values", "resource_ids"])
        for n in sorted(missions):
            rec = missions[n]
            for t in sorted(rec["spawns"]):       # hw7 resource catalog
                w.writerow([n, "spawn_hw7", t, rec["spawns"][t], "", "", ""])
            for t in sorted(rec["logical"]):      # hw3 dispatch type (joins INIT)
                init = rec["init"].get(t)
                w.writerow([n, "spawn_logical_hw3", t, rec["logical"][t],
                            f"0x{init:08x}" if init else "(no INIT)",
                            " ".join(map(str, rec["hp"].get(t, []))),
                            " ".join(map(str, rec["res_by_logical"].get(t, [])))])
            for t in sorted(rec["init"]):
                w.writerow([n, "objobj_init", t, "",
                            f"0x{rec['init'][t]:08x}", "", ""])


def write_type_csv(missions, agg, path):
    import csv
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["type_hw7", "name", "n_missions", "total_instances",
                    "missions", "has_objobj_init_same_id", "note"])
        # An objobj INIT entry exists for the SAME small id in any mission that
        # also spawns this hw7 (only meaningful for the overlapping low ids).
        for t in sorted(agg):
            a = agg[t]
            ms = sorted(a["missions"])
            same_id = sorted(n for n in ms
                             if t in missions[n]["init"])
            note = ""
            if t > 15:
                note = "global resource id; outside INIT table range"
            elif not same_id:
                note = "no objobj INIT slot with this (low) id in spawning missions"
            w.writerow([t, TYPE_NAMES.get(t, ""), len(ms), a["total"],
                        " ".join(map(str, ms)),
                        " ".join(map(str, same_id)), note])


def write_md(missions, agg, path):
    lines = []
    lines.append("# AC1 master OBJECT-TYPE catalog (Phase 5)\n")
    lines.append("Generated by `tools/mission/type_catalog.py`. Two type "
                 "namespaces are tracked separately (see the namespace note in "
                 "the tool docstring / `scratch/re/type_catalog_notes.md`).\n")

    parsed = [n for n in missions if missions[n]["has_stream"]]
    with_obj = [n for n in missions if missions[n]["has_objobj"]]
    nospawn = [n for n in missions if missions[n]["has_stream"]
               and not missions[n]["has_spawn_chunk"]]
    lines.append("## Coverage\n")
    lines.append(f"- missions with a chunk stream (entry 2N+1): "
                 f"{len(parsed)}  -> {parsed}")
    lines.append(f"- missions with an objective object (entry 2N): "
                 f"{len(with_obj)}")
    lines.append(f"- streams WITHOUT a chunk-12 spawn table: {nospawn}")
    lines.append(f"- distinct global hw7 spawn TYPE ids: {len(agg)} "
                 f"(range {min(agg)}..{max(agg)})\n")

    lines.append("## Global spawn TYPE ids (chunk-12 hw7), most-used first\n")
    lines.append("| type_hw7 | n_missions | total_instances | missions |")
    lines.append("|---|---|---|---|")
    for t in sorted(agg, key=lambda x: (-agg[x]["total"], x)):
        a = agg[t]
        ms = " ".join(map(str, sorted(a["missions"])))
        lines.append(f"| {t} | {len(a['missions'])} | {a['total']} | {ms} |")

    lines.append("\n## Objective-object INIT tables (per mission, logical types)\n")
    lines.append("Logical type id -> INIT fn ptr (absolute 0x801Cxxxx, in the "
                 "per-mission objective-object code region).\n")
    lines.append("| mission | logical types | INIT pointers |")
    lines.append("|---|---|---|")
    for n in sorted(with_obj):
        init = missions[n]["init"]
        if not init:
            lines.append(f"| {n} | (none) | |")
            continue
        ts = " ".join(str(t) for t in sorted(init))
        ps = " ".join(f"{t}=0x{init[t]:08x}" for t in sorted(init))
        lines.append(f"| {n} | {ts} | {ps} |")

    lines.append("\n## Logical type (hw3) <-> INIT join + HP (per mission)\n")
    lines.append("| mission | hw3 | count | HP (hw11) | resources (hw7) | INIT[hw3] |")
    lines.append("|---|---|---|---|---|---|")
    for n in sorted(parsed):
        rec = missions[n]
        for t in sorted(rec["logical"]):
            init = rec["init"].get(t)
            lines.append(
                f"| {n} | {t} | {rec['logical'][t]} | "
                f"{','.join(map(str, rec['hp'].get(t, [])))} | "
                f"{','.join(map(str, rec['res_by_logical'].get(t, [])))} | "
                f"{'0x%08x' % init if init else '(none)'} |")

    lines.append("\n## Anomalies / flags\n")
    # logical types (hw3) that are spawned but have NO INIT[hw3] in that mission
    # type 0 = no-op stub, -1 = none; only flag real (>=1) types lacking an INIT
    miss_join = sorted(
        (n, t) for n in parsed for t in missions[n]["logical"]
        if t >= 1 and t not in missions[n]["init"])
    lines.append(f"- spawned logical types (hw3>=1) with NO INIT[hw3] in the same "
                 f"mission: {miss_join if miss_join else 'none (clean)'}")
    # spawn types with no matching low-id objobj init anywhere
    low_spawn = {t for t in agg if t <= 15}
    no_init = sorted(t for t in low_spawn
                     if not any(t in missions[n]["init"]
                                for n in agg[t]["missions"]))
    lines.append(f"- (hw7 resource ids <=15 with no same-id INIT anywhere — expected, "
                 f"different namespace): {no_init}")
    high = sorted(t for t in agg if t > 15)
    lines.append(f"- spawn ids > 15 (outside the INIT table window; expected, "
                 f"these are global resource ids): {len(high)} ids "
                 f"({min(high) if high else '-'}..{max(high) if high else '-'})")
    init_only = sorted(n for n in with_obj
                       if missions[n]["init"] and not missions[n]["spawns"])
    lines.append(f"- missions with an INIT table but no spawn types parsed: "
                 f"{init_only}")
    lines.append("")
    with open(path, "w") as f:
        f.write("\n".join(lines))


def dump_mission(missions, n):
    if n not in missions:
        print(f"mission {n}: no FDAT entries present")
        return
    rec = missions[n]
    print(f"=== mission {n} ===")
    print(f"stream entry {2*n+1:03d}.bin present: {rec['has_stream']} "
          f"(chunk-12: {rec['has_spawn_chunk']})")
    print(f"objobj entry {2*n:03d}.bin present: {rec['has_objobj']}")
    print()
    print("logical dispatch type (hw3) -> count | HP(hw11) | resources(hw7) | INIT[hw3]:")
    if rec["logical"]:
        for t in sorted(rec["logical"]):
            init = rec["init"].get(t)
            print(f"  hw3 {t:3d}  x{rec['logical'][t]:<3d} "
                  f"HP={rec['hp'].get(t, [])} res={rec['res_by_logical'].get(t, [])} "
                  f"INIT={'0x%08x' % init if init else '(none)'}")
    else:
        print("  (none)")
    print()
    print("chunk-12 resource ids (hw7) -> instance count:")
    if rec["spawns"]:
        for t in sorted(rec["spawns"]):
            print(f"  hw7 {t:4d}  x{rec['spawns'][t]}")
    else:
        print("  (none)")
    print()
    print("objective-object INIT table (+0x20 + logical_type*4):")
    if rec["init"]:
        for t in sorted(rec["init"]):
            print(f"  logical type {t:2d}  INIT = 0x{rec['init'][t]:08x}  "
                  f"(file_off 0x{rec['init'][t]-OBJOBJ_BASE:x})")
    else:
        print("  (none)")


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--fdat", default=DEFAULT_FDAT,
                    help="dir of extracted FDAT entries (NNN.bin)")
    ap.add_argument("--mission", type=int,
                    help="dump just this mission to stdout (no files written)")
    args = ap.parse_args()

    missions = collect(args.fdat)

    if args.mission is not None:
        dump_mission(missions, args.mission)
        return

    agg = build_type_agg(missions)
    write_long_csv(missions, OUT_LONG)
    write_type_csv(missions, agg, OUT_CSV)
    write_md(missions, agg, OUT_MD)

    parsed = [n for n in missions if missions[n]["has_stream"]]
    with_obj = [n for n in missions if missions[n]["has_objobj"]]
    print(f"missions parsed (stream present): {len(parsed)}")
    print(f"missions with objective object:   {len(with_obj)}")
    print(f"distinct global hw7 spawn types:  {len(agg)} "
          f"(range {min(agg)}..{max(agg)})")
    print(f"wrote {OUT_CSV}")
    print(f"wrote {OUT_LONG}")
    print(f"wrote {OUT_MD}")


if __name__ == "__main__":
    main()
