# Sub-trace D — spawn→geometry binding (live, training mission, 2026-06-11)

Source: DuckStation, pristine/backup disc, save slot 1 (training mission). PA file =
P0\PA00.T (stage byte `0x8004121B = 0x00`). Artifacts: `block_table_8019F538.bin`,
`inst_table_8019FAB8.bin`, `parse_tables.py`.

## Confirmed chain (matches REFERENCE.md §11 / docs/PA_HEADER.md / MISSION_SYSTEM.md)

```
instance/spawn table 0x8019FAB8 (stride 44, 256 slots)
   record[+0x0a] = BLOCK INDEX  (empty slot marker: first u16 == 0xffff)
        │  FUN_80078B14
        ▼
block record table 0x8019F538 (stride 44)
   blkrec = 0x8019F538 + block_index*44
   blkrec[+0x00] = half[geom+4] >> 2   (element count A)
   blkrec[+0x02] = half[geom+6] >> 2   (element count B)
   blkrec[+0x28] = GEOMETRY work-RAM pointer  ──► transformed geometry block
```

## Training-mission live values
Block table — exactly **2** active records:
| blk | +0x00 | +0x02 | +0x28 geom ptr | role |
|-----|-------|-------|----------------|------|
| 0 | 100 | 212 | 0x801a42c0 | stage / environment (from PA entry-1 registration) |
| 1 | 120 | 360 | 0x801a3db8 | the MT model (training dummies) |

Geometry-header cross-check (counts ARE `half[geom+4]>>2` / `half[geom+6]>>2`):
- 0x801a42c0: +4=0x0190(400→100 ✓)  +6=0x0352(850→212 ✓)
- 0x801a3db8: +4=0x01e0(480→120 ✓)  +6=0x05a0(1440→360 ✓)

Instance/spawn table — active spawns (all others = 0xffff empty):
| ins | x | y | z | +0x0a block | note |
|-----|---|---|---|-------------|------|
| 0,1,2 | ~0 | 0 | 0 | 0 | stage/world placement (block 0) |
| 4 | 257 | 5200 | -26 | 1 | enemy MT #1 |
| 5 | 513 | 5200 | 5518 | 1 | enemy MT #2 |

→ The two on-screen training MTs are ins[4]/ins[5], both block index 1 (same model,
y=5200 floor, different x/z). Binding check: every active spawn's block index
resolves to a VALID work-RAM geometry pointer. CONFIRMED.

## OPEN — block_index → PA file entry (the Phase-3 viewer bridge)
The block-record count fields (`+4/+6 = count<<2`) are **runtime-computed and do NOT
appear in the raw PA file headers** (scan for matching +4/+6 in PA00 → no hit). So a
viewer cannot map block index → PA entry by header match. The mapping is built at load
time: blk[0] from PA entry-1; blk[1..N] from the mission **chunk-11** stream
(FUN_800739AC). To wire AC1mod (Phase 3): walk the mission chunk stream
(`tools/mission_parse.py`) to recover, per block index, which PA entry / sub-object set
the geometry came from, then place spawns (instance pos + block→geometry) in the scene.
