#!/usr/bin/env python3
"""
t_repack_level_test.py — Stage 1 of the level round-trip validation.

Proves tools/t_repack.py survives a real, semantically-meaningful LEVEL edit:
extract Rescue Transport Truck (FDAT entry 67), MOVE one placed section by
rewriting its chunk-7 translation (size-preserving), reimport with t_repack, and
assert the round-trip is correct AND the move is real.

The edit (DECIDED): RTT chunk-7 record 2 (block 8, translation (1154,0,531))
displaced by +8000 X, +4000 Y -> (9154,4000,531). 6 bytes changed.

Outputs (gitignored, game-derived):
  disc_map/levels_assembled/rtt_baseline_topdown.png
  disc_map/levels_assembled/rtt_moved_topdown.png
  scratch/fdat_rtt_moved.T     <- flat FDAT for Stage 2 (psxinject)
"""
import sys, os, struct

HERE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(HERE, "tools", "extract"))
sys.path.insert(0, os.path.join(HERE, "disc_map", "trace"))
import t_repack as T
import assemble_levels as AL

ENTRY      = 67                 # RTT level (mission 33)
REC        = 2                  # chunk-7 record to move (block 8; also instanced
                                # by rec 48, which stays put -> proves it's the
                                # PLACEMENT record that moved, not the shared block)
# PSX Y is DOWN (level Y-extent [-28572, 0], 0 = floor). So a LIFT is -Y.
DELTA      = (8000, -6000, 0)   # +X (top-down visible) + lift up (in-game float)
REC_STRIDE = 52
TRANS_OFF  = 0x10               # s16[3] translation within a record

OUT_PNG = os.path.join(HERE, "disc_map", "levels_assembled")
SCRATCH = os.path.join(HERE, "scratch")
FLAT_OUT = os.path.join(SCRATCH, "fdat_rtt_moved.T")


def moved_section_world(entry, rec_idx):
    """World-space vertices of the section placed by chunk-7 record `rec_idx`."""
    blocks = AL.blocks_of(entry)
    plc = AL.placements_of(entry, len(blocks))      # [(blk, p2, light), ...]
    blk, p2, _ = plc[rec_idx]
    bv, _ = AL.block_mesh(blocks[blk])
    return blk, p2, [(x + p2[0], y + p2[1], z + p2[2]) for (x, y, z) in bv]


def main():
    os.makedirs(OUT_PNG, exist_ok=True)
    os.makedirs(SCRATCH, exist_ok=True)
    fm = T.load_filemap()
    rec_f = T.find_file(fm, "GG/COM/FDAT.T")
    flat = T.read_flat_from_disc(fm, rec_f)
    c = T.unpack(flat)
    orig_entries = list(c.entries)
    e = orig_entries[ENTRY]
    print(f"FDAT entry {ENTRY}: {len(e)//2048} sectors, checksum {'OK' if T.verify(e) else 'BAD'}")

    # --- locate the record + capture the section's world mesh BEFORE the edit ---
    ch = AL.chunks_of(e)
    toff = ch[7][0]
    rec_abs = toff + REC * REC_STRIDE + TRANS_OFF
    p2_before = struct.unpack_from("<3h", e, rec_abs)
    blk, p2_chk, world_before = moved_section_world(e, REC)
    print(f"  rec{REC}: block {blk}, translation {p2_before} ({len(world_before)} verts)")
    assert p2_before == p2_chk

    # --- edit: rewrite the 6-byte translation in a copy of the entry ---
    ba = bytearray(e)
    new_p2 = tuple(p2_before[i] + DELTA[i] for i in range(3))
    struct.pack_into("<3h", ba, rec_abs, *new_p2)
    print(f"  edit -> translation {new_p2} (delta {DELTA}, 6 bytes)")

    # --- reimport via t_repack (drop old checksum word; replace_entry refixes) ---
    T.replace_entry(c, ENTRY, bytes(ba)[:-4])
    out = T.pack(c)
    c2 = T.unpack(out)
    ep = c2.entries[ENTRY]

    # --- assertions: the round-trip gate ---
    checks = {
        "edited entry checksum valid": T.verify(ep),
        "edited entry size unchanged": len(ep) == len(e),
        "FDAT total size unchanged":   len(out) == len(flat),
        "only entry 67 changed": (
            ep != orig_entries[ENTRY] and
            all(c2.entries[i] == orig_entries[i]
                for i in range(len(orig_entries)) if i != ENTRY)),
    }

    # --- assertion: the section actually moved by exactly DELTA ---
    _, p2_after, world_after = moved_section_world(ep, REC)
    moved_ok = (p2_after == new_p2 and len(world_after) == len(world_before) and
                all(world_after[k][a] == world_before[k][a] + DELTA[a]
                    for k in range(len(world_before)) for a in range(3)))
    checks["section moved by exact delta"] = moved_ok

    # --- renders + Stage-2 flat image ---
    Vb, gb, _, _ = AL.assemble(ENTRY, e)
    Vm, gm, _, _ = AL.assemble(ENTRY, ep)
    AL.topdown_png(os.path.join(OUT_PNG, "rtt_baseline_topdown.png"), Vb, gb)
    AL.topdown_png(os.path.join(OUT_PNG, "rtt_moved_topdown.png"), Vm, gm)
    with open(FLAT_OUT, "wb") as f:
        f.write(out)

    print("\n=== Stage 1 assertions ===")
    for k, v in checks.items():
        print(f"  [{'PASS' if v else 'FAIL'}] {k}")
    def bbox(verts):
        return tuple((min(v[a] for v in verts), max(v[a] for v in verts))
                     for a in range(3))
    print(f"\n  moved section world bbox (X,Y,Z):")
    print(f"    before {bbox(world_before)}")
    print(f"    after  {bbox(world_after)}")
    print(f"  renders -> {OUT_PNG}/rtt_baseline_topdown.png, rtt_moved_topdown.png")
    print(f"  Stage-2 flat FDAT -> {FLAT_OUT} ({len(out)//2048} sectors)")

    ok = all(checks.values())
    print(f"\nSTAGE 1: {'PASS' if ok else 'FAIL'}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
