# AC1 mission scripting — the secondary actor-thread VM (chunk 4)

Target **SLUS-01323 (v1.1)**. Static RE (offline disasm of
`overlays/mission_overlay_FDAT202_80050000.bin` @ `0x80050000`). This is the
**authoring reference** for scripted mission behaviour. Tooling: round-trippable
(dis)assembler `tools/mission/mission_script.py` (`--selftest` = 56/56 byte-exact).
Companion: `MISSION_SYSTEM.md` (the *primary* objective-command VM `FUN_8008A0B0`),
`ENTITY_TYPES.md` (objective object + conditions).

AC1 has **two** mission VMs:
1. **Primary objective-command VM** `FUN_8008A0B0` (opcodes 1..10, jump table
   `0x8004C164`) — see `MISSION_SYSTEM.md` §4.
2. **Secondary actor-thread VM** (this doc) — a per-frame interpreter that drives
   timed actor "threads": scripted spawns, cutscene entrances, moving/lerping objects,
   timed sound/marker triggers, and the data-driven mission-result set.

## Architecture (CONFIRMED)

- **Script blob = chunk index 4** of the mission chunk stream (FDAT entry `2N+1`).
  Small (32–112 B), present in every mission. RAM pointer kept at `0x801D0B64`.
- Launched by **primary VM command 2** (`FUN_8008A158` → `FUN_8008C2FC` →
  `vm_load_threads 0x8008C178`), which loads a *set* by index — so cmd 2's argument
  selects which wave/phase of threads becomes active (multi-set blobs exist, e.g.
  mission 8 = 3 sets).
- A set expands into N **threads**; the loader builds a 24-byte record per thread at
  `0x801D0B60` (count `0x801D0B51`).
- Each frame, `vm_tick 0x8008B380` iterates threads; per running thread the dispatch
  loop `0x8008B42C` executes opcodes until it blocks (`set_loop`/`mark_lerp`) or `END`.
- Dispatch is a hand-written binary search (not a jump table). The opcode u16 is
  consumed (`PC += 2`) before the handler; **unknown opcodes are silent 0-operand
  no-ops in v1.1** (verified: all 106 threads on the disc walk cleanly to `0xFFFF`).

## On-disc blob format (CONFIRMED, byte-verified)

```
blob:
  u16 set_offset[NSETS]        ; NSETS = set_offset[0]/2
  ... set headers + thread bytecode ...
set header (at blob + set_offset[s]):
  u16 flags            ; +0 (0/1; HYP autostart/loop)
  u16 thread_count     ; +2  -> 0x801D0B51
  u16 extra            ; +4  -> 0x801D0B52 (HYP frame budget; 120/0)
  u16 thread_off[thread_count]  ; +6 (byte offsets within blob)
thread (at blob + thread_off[i]):
  u16 thread_hdr       ; +0  (scratch-size selector: 0->16B, 1->530B, 2->40B); PC = this+2
  ... opcodes ...
  u16 0xFFFF           ; END
```

Loader `vm_load_threads(set_idx)` @ `0x8008C178`; tick globals live in the world page
`0x801D0Bxx` (`B50`=set idx, `B51`=thread count, `B60`=thread table, `B64`=blob base).

## Opcode table (CONFIRMED from disasm unless noted)

`OPLEN` = operand bytes after the 2-byte opcode; operands are LE int16 unless noted.
`s3` = the thread's actor scratch (record `+0x14`); `unit` = bound target object.

| op | mnemonic | OPLEN | operands | semantics |
| --- | --- | --- | --- | --- |
| `0x0000` | copy_pos_lo | 6 | x,y,z | snapshot pose→track1; arms position-lerp (target track) |
| `0x0002` | copy_pos_hi | 6 | x,y,z | second track |
| `0x0008` | spawn_at8 | 6 | a1,a2,a3 | `FUN_8004E98C(unit+100,…)` place/spawn helper |
| `0x0009` | spawn_at9 | 6 | a1,a2,a3 | same entry |
| `0x000A` | init_unit | 12 | (skip),5×hw | init bound unit block (`unit+0x65/74/76/78/7A/7C/15C`); **field meanings HYP** |
| `0x000B` | subcall_b320 | 0 | — | `FUN_8008B320` re-run/restart actor sub |
| `0x000E` | mark_lerp | 14 | 7×hw | inline pose/track block consumed by lerp tail; **field layout HYP** |
| `0x000F` | op_f | 2+cnt*4 | u16 cnt, cnt×(2 hw) | count-prefixed list (lerp tail spawns cnt markers/projectiles) |
| `0x1000` | sel_actor | 2 | idx | bind subsequent ops to thread-record/unit `idx` (`0x801D0B60+idx*24`) |
| `0x1001` | set_xyz | 6 | x,y,z OR (0x8000,refidx,_) | literal XYZ into `s3`, or copy pose from actor-table[refidx] (`0x801A8B08+refidx*100`) |
| `0x1002` | set_xyz2 | 6 | x,y,z OR ref | second XYZ/ref form |
| `0x1003` | spawn_group | 2 | N | spawn template N (`0x8019FAB8+N*44` → `FUN_80078CFC`); binds `unit=0x801A26B8+slot*0x170` |
| `0x1004` | play_sound | 2 | snd | `FUN_800529F0(0,0,snd)` |
| `0x1005` | flag_all | 0 | — | OR `0x8000` into `obj+0x32` for active entities (HYP: reveal/targetable) |
| `0x1006` | clear_targets | 0 | — | snapshot+disable the radar/target list |
| `0x1007` | call_db18 | 2 | a0 | `FUN_8004DB18(a0)` (trigger/state in lower block) |
| `0x1008` | set_obj_flag | 6 | lo,?,val | write byte `+6=val` on a world-object id range (`0x801D0BA0`) |
| `0x1009` | spawn_marker | 6 | a,kind,c | pop a `#Location`/COM marker/text on objective objects |
| `0x100A` | **set_result** | 2 | flags | `FUN_8004C318(flags)` → `0x8019F524` (`0x100` success / `0x200` fail / `0x80` progress) |
| `0x100B` | set_obj_word | 4 | idx,val | `sh val,[0x801A8B24+idx*100]`; then `FUN_80073170()` |
| `0x100C` | despawn_group | 2 | N | mirror of spawn_group: `FUN_80078D78` despawn entity for template N |
| `0xFFF0` | set_loop | 2 | n | **blocking**: run the action/lerp tail for n frames (state=1) |
| `0xFFFF` | END | 0 | — | terminator (arms final lerp if pending) |
| other | (noop) | 0 | — | any non-table u16 (`0x000C/0x000D/0x0010/…`) = reserved 0-operand no-op in v1.1 |

The **action/interpolation tail** (thread state≠0) runs scheduled actions keyed by an
"opcode-seen" bitmask, computing a fixed-point lerp parameter `(elapsed<<12)/total`;
bit→callee map is read out at `0x8008BC04–0x8008C078` (e.g. `0x1`=position interp via
`FUN_800276F4`, `0x80000`=`0x8019F524|=0x400`+`FUN_800796F4`). Per-bit timing units HYP.

## Authoring (assembler spec)

`tools/mission/mission_script.py` parses chunk 4 → `Blob→Set→Thread→Op`, disassembles,
and reassembles **byte-exact** (`--selftest` 56/56). To author:
1. Emit set header `{flags, thread_count, extra, thread_off[]}` then each thread
   `{thread_hdr, opcodes…, 0xFFFF}`; prepend `set_offset[]`; back-patch offsets.
2. Encode each op as `u16 opcode` + `OPLEN` bytes (per table). `0x000F` is variable
   (`cnt`, then `cnt*4`). `set_xyz`/`set_xyz2` always emit 3 hw. Pass reserved words
   through verbatim.
3. Re-wrap as chunk 4 `[u32 len][blob]` in the `2N+1` stream, repack the FDAT entry
   (`t_repack.replace_entry` recomputes the checksum), reinject (`tools/disc/reinject.py`).

## Open / confirm-live
- `init_unit`/`mark_lerp`/`op_f` inline-record field meanings (HP/AI/anim names).
- Set-header `flags`/`extra` precise meaning; per-action lerp timing units.
- Writer of `0x801D0B64` (which chunk-stream handler stages chunk 4) — lower overlay/base EXE.
- Ground-truth: breakpoint `0x8008B830`/`0x8008B42C` to confirm live thread order + `init_unit`.
