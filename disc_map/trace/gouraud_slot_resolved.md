# Gouraud 0x34/0x3c vertex-index slot — RESOLVED (live trace, 2026-06-11)

Source: live DuckStation, pristine/backup disc (SLUS-01323 v1.1), training mission
(save slot 1). Relocation jump table `0x8004B184` (mission/geometry overlay loaded).

## Method
Read the relocation-walker handler jump table at `0x8004B184` (word stride,
index = `(type & 0xBC) - 0x20`), then disassembled the per-type handlers from the
running game. Each handler runs once at geometry load: it rewrites index halfwords
in the record in place. `a1 = record + 4`. A halfword shifted:
- `<<4` (×16) = **vertex-position index** (into the transformed-vertex pool, stride 16)
- `<<3` (×8)  = **normal/colour index** (into the normal pool, stride 8)

Calibrated against the CONFIRMED textured-tri `0x24` (handler `0x80057640`): its
`<<4` slots are `a1+0xe,0x10,0x12` = rec+0x12,0x14,0x16 = the known contiguous 3
vertices. Frame confirmed.

## Handler addresses (from table @ 0x8004B184)
| type | index | handler |
|------|-------|---------|
| 0x20 flat tri | 0 | 0x800575c0 |
| 0x24 tex tri | 4 | 0x80057640 |
| 0x28 flat quad | 8 | 0x800576c0 |
| 0x2c tex quad | 12 | 0x80057764 |
| **0x34 gouraud tri** | 20 | **0x80057674** |
| **0x3c gouraud quad** | 28 | **0x800577a4** |
| (default no-op) | — | 0x80057574 |

## RESULT — gouraud index layout (record-relative; record base = byte0 tag)
Gouraud records interleave **[normal_idx, vertex_idx]** pairs (stride 4):

- **0x34 gouraud tri** (reclen 28): index region rec+0x10..0x1c
  - normals: rec+0x10, 0x14, 0x18
  - **vertices: rec+0x12, 0x16, 0x1a (count 3, stride 4)**
- **0x3c gouraud quad** (reclen 36): index region rec+0x14..0x24
  - normals: rec+0x14, 0x18, 0x1c, 0x20
  - **vertices: rec+0x16, 0x1a, 0x1e, 0x22 (count 4, stride 4)**

Reclen cross-check: tri 0x04(header)+0x0c(shading 0x04-0x0f)+0x0c(6 idx hw) = 0x1c=28 ✓;
quad 0x14(header+shading)+0x10(8 idx hw) = 0x24=36 ✓.

## Why the old decode dropped 5–30% of faces
`pa_parser` read gouraud verts as CONTIGUOUS uint16 (stride 2) from 0x14. Real
layout is stride 4 (a normal index sits between each vertex index). Stride-2 reads
picked up ×8 normal-pool indices as vertices; those exceed vtx_count → out-of-range
→ dropped. Fix: per-type stride. 0x34 → (first=0x12, n=3, stride=4);
0x3c → (first=0x16, n=4, stride=4). Other types keep stride 2.

Bonus: gouraud records carry a per-vertex NORMAL index (normal = vertex_offset − 2),
confirming smooth per-corner lighting on these primitives.
