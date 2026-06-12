# Sub-trace B — mission → stage byte + sub-resource map (IN PROGRESS)

The 9 GameState bytes that drive a stage load (read live in-mission; they persist
through the mission). Stage byte `0x8004121B` → main `P{n}\PA{nn}.T`; sub-resource
selectors `0x121D..0x1224` → entry = byte + addend[slot] (see subresource_loader_C.md;
addends 0:+2 1:+10 2:+44 3:+88 4:+60 5:+70 6:+108 7:+142; 0xFF = empty).

| mission | stage 0x121B | PA file | sub-res 0x121D..0x1224 | decoded entries |
|---------|-------------|---------|------------------------|-----------------|
| Training | 0x00 | PA00 | 00 03 05 01 00 01 00 18 | e2,e13,e49,e89,e60,e71,e108,e166* |
| Eliminate Squatters | 0x00 | PA00 | 00 03 01 01 00 03 ff ff | e2,e13,e45,e89,e60,e73,—,— |
| Eliminate Strikers | 0x00 | PA00 | 00 03 01 01 00 0c ff ff | e2,e13,e45,e89,e60,e82,—,— |

(* training slot7 = 0x18+142 = e166, slightly past PA00's 176 entries — fine.)

Observations:
- Early Chrome missions REUSE the same stage (PA00); they differ only in the
  sub-resource selectors (which MTs/objects/effects spawn).
- Squatters vs Strikers differ in exactly ONE byte: slot 5 (e70 category) 0x03→0x0c
  (e73 vs e82). Clean independent confirmation of the item-C addend model.

TODO (full B sweep): load all 50 missions, record these 9 bytes each, to build the
complete mission→stage map. Now semi-automatable: trap the loader (read-watch
0x8004121D) or just snapshot 0x8004121B..0x1224 once in each mission.
