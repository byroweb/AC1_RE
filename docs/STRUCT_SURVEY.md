# Struct survey: the layouts matched sources declare locally

Matching agents may not edit `include/ac1`, so each layout an agent recovered went into its
matched C file as a local typedef. This page lists those layouts, says which ones are now
shared headers, and compares type-recovery tools for this kind of work.

`python tools/match/structsurvey.py table` writes the two tables at the end of this page.
`structsurvey.py merge ADDR` shows every name each source gave each offset of one global. It
is the view used to build the headers below. See the Struct survey section in
`tools/match/README.md`.

**Numbers (2026-10-26):** 2,150 matched sources contain 2,539 local struct and union
definitions. 410 global addresses have a local struct bound to them, and 154 of those are
bound in two or more sources. A further 187 layouts recur, by shape, in two or more sources
without a fixed address.

## Consolidated in this change

| global | shared type (header) | local declarations merged | settled |
| --- | --- | --- | --- |
| 0x80040FA0 player blocks ×2, `*0x80039D18` current, `*0x80039D30/34` versus sides | `VsPlayer` extended (`vsplayer.h`); `g_players`, `g_player`, `g_vs_sides` | 44 sources through 0x80039D18, 17 at 0x80040FA0 and 20 through 0x80039D30/34, under ~20 names (`Pilot`, `Side3`, `CSide3`, `Player`, `PlRec`, `VRec4`, `WSide4`, ...) | +0x084 `TargetSlot targets[8]` with +0x080 `target` (ovl202's `g_target_head` / `g_target_list` are this block's first copy); +0x290 is a twelve-entry u16 key map (`key_up`, `key_down`, then the existing `turn_l`/`turn_r`/`mask_r`/`mask_l`); +0x2DA three `VsWeapon` (0x3C) slots; +0x394 the AC entity, +0x398 the camera block; +0x21C `kick_scale` (ovl202 `D_800411BC` × `WeaponSpec.kick` >> 8). Existing field names were kept; other names the sources used are in the comments |
| 0x801A2550 front-end pad block | `PadState` 0x2C (`frontend.h`); `g_fe_pad` | 35 sources (`PadState` ×25, `PadRepeat` ×8, `PadSt`, `FePad`), 0x20 or 0x28 bytes, plus the scalars `D_801A2554..79` | Widened to 0x2C from the poll `func_8005B4B8` and the reset `func_80050CE0`. +0x04/+0x06 are the repeat period and the first-repeat delay (the sources disagreed on which is which; the poll counts +0x06 down first). +0x14 holds the raw buttons, +0x1C last frame's (most sources called it `held`), +0x20 the pad-swap flag, +0x24 the repeat divisor |
| 0x80031A58 equipped part ids | `g_ac_parts` u8[12], `AC_PART_*` (`frontend.h`) | scalars in ~30 ovl201 sources, `Raw12` ×2 | Slots 0 core, 2 head, 3 arms, 8 generator and 11 legs each have two or more sources (the part table each is read with); booster (1), FCS (9) and the weapon slots (6, 7) have one source each, so they are only in a comment. `func_80071A18` calls slot 3 `legs`; its own `legs < 8` test is the weapon-arms rule, so slot 3 is arms |
| VM threads (`g_mission_ctl.vm_thread_tab`) | `VmThread` 0x18, `VmState` 0x28 (`vm.h`) | `func_8008B380`, `func_8008C178` | +0x0C is the thread kind that picks the work-area size (16, 530 or 40 = `VmState`) |
| 0x80198B08 map objects ×512, list 0x8019F308, count 0x8019F508 | `MapObj` 0x34 (`vm.h`); `g_map_objs`, `g_map_obj_list`, `g_map_obj_count` | 26 sources (`VmNode`, `WObj`, `Zone`, `Slot34`, `MapMark`, `Node`, fdat `*Node`) | Object kinds read +0x10..+0x17 differently: drawn objects as a scale `SVECTOR`, destructibles as +0x14 handler. They are a union. Map markers read +0x22 as a name. The code indexes 0x34-byte records; `ENTITY_TYPES.md`'s 0x6C row stride is still open (LIVE_CHECKLIST ENT-10) |
| 0x8019FAB8 spawn templates ×256 | `SpawnTmpl` 0x2C, `SpawnBlock` (`vm.h`); `g_spawn_tmpl` | 19 sources (`VmSpawn`, `Slot`, `Slot2C`, `Rec2C`, nine `World`-style blocks that run on into `g_ac_entities`) | +0x04.. are the spawn record's halfwords hw0..hw19 (from `func_80073B74`), named where the docs or code say what they hold: pos, kind (-1 = none), section, flags, map-object index, HP seed, world-struct index |
| 0x8008E658 weapon spec ×7, 0x8008E610 spread ×9 | `WeaponSpec` 6, `WeaponSpread` 8 (`projectile.h`); `g_weapon_specs`, `g_weapon_spreads` | ovl202 `func_8007D2F8` / `func_8007D068`, fdat000 `func_801C7DA8` / `func_801C88EC` | Row counts read from the retail ovl202 bytes. Spec row 7 is other data; the rows are range 500/600, kick 32/256, trigger, cool-down 2/36 |
| 0x800B91F4..0x800BAFCC part-spec tables (10 categories) | `PartHdr` 0x28 prefix, `PART_CAT_*` (`frontend.h`); `g_parts_head`, `g_parts_core`, ... `g_parts_gen` (u8[]) | reported from the wave-13 match of ovl201 `func_80075898` (not yet on main), plus 5 matched sources | Every row opens with two 18-byte names and an s32 price at +0x24 (checked against the retail rows: "XCA-00" / "CORE UNIT" / 61500). Strides differ per category (44..96), so the tables stay u8[] and the stride table is in the header. Not modelled yet: the 24-byte weapon stat records at 0x800BB300 and the string-pointer tables 0x800BBA34..0x800BBAD8 |
| 0x1F800000 scratchpad | `ViewSpad` (`ovl202_view.h`) for the ovl202 world-object view only | 81 sources, about 50 layouts | Not one structure. Each routine (and its copies in the other overlays) keeps a private scratch layout. Only `func_8008CC88` and `func_8008D3B0` share this one |

`vsplayer.h`, `frontend.h`, `vm.h` and `ovl202_view.h` are **opt-in**: `ac1.h` does not
include them, because names like `PadState`, `VmThread` and `Pilot` are already local
typedefs in matched sources. A matched file is never edited, so pulling those names into the
umbrella would break it. `ctx.py gen` now declares a global whose type lives in an opt-in
header inside a generated block at the end of that header. `ctx.py check` compiles each
opt-in header after `ac1.h`, and `ctx.py m2c` / `context` add the headers that own symbols
(or any header named with `--with`). With the new symbols, m2c prints the pad poll as
`g_fe_pad.first -= 1` and `g_fe_pad.press = g_fe_pad.now & ~g_fe_pad.latch`.

## Tool evaluation: what can infer these layouts?

**Ghidra 12.1.4 (pristine project, headless through PyGhidra; GhidraMCP needs the GUI).**

- *Fixed-address blocks* such as the pad block, the parts array and the map-object and
  spawn tables: the decompiler prints separate globals (`DAT_801a2554`, `DAT_801a2556`, ...,
  `DAT_801a2570`). Nothing in Ghidra groups globals at fixed addresses into a structure.
  Every global in this change is of that kind.
- *Auto Fill-in Structure* (`FillOutStructureHelper`) on a pointer variable works on one
  function at a time. On `func_80078D84`'s object pointer it gave a 0xB2-byte structure
  with 13 typed fields at +0x84..+0xB0. These are the offsets and widths that function
  touches. It gave no names and left untyped words undefined. The agent's `AsmWork` in the
  same file has those offsets plus the pointer fields and names. Merging 44 views of the
  pilot record would still be a manual union over 44 such runs.
- The project holds the exe and the ovl201 block only. Overlays 202-204 and the mission
  overlays would have to be imported before Ghidra could see the pilot record, the VM or the
  map-object and spawn users at all.

**m2c** infers no structures. It prints raw offsets, or field names when a context supplies
the types, which is the output of consolidation, not the input. **Wyrelade's headers** stay a
size cross-check (see `include/ac1/README.md`). They cover overlays 201/202 objects, not
these globals. **`tools/duckstation/live_probe.py`** confirms what a field holds in a running
game. It settles disputes but does not recover layouts.

**Recommendation: none of these beats hand consolidation, so do it with
`structsurvey.py merge`.** 5,765 of 5,769 functions are matched, so the matched C is the
ground truth for every access width and signedness. Type propagation can only rediscover a
subset of that, with no names. The work that remains is reconciling names and settling
conflicts: which of two adjacent s16s is counted down first, a union versus two object
kinds, a mislabelled slot. That needs the code and the docs. `merge` puts every source's
claim for each offset on one screen, with file counts. Use `live_probe.py` when the code
alone cannot settle what a field means. Ghidra's Auto Fill-in Structure is still useful
before a function is matched, on an unknown pointer argument, but almost none of those are
left.

## Next candidates (not done here)

By source count, these global layouts recur and have no shared type yet. Each would get the
same treatment:

- 0x801EF6C8 (42 sources, ovl201): `{u8 idx; u8 *prim}` primitive-buffer cursor (`GfxBuf`, `UiGfx`).
- 0x8017E500 / 0x8017E508 (17 + 8, ovl201): TMD packet cursors (`TmdPktF3`, `TmdPktG3`, ...).
- 0x8019C090 (16, ovl201): parts-shop state `ShopState`.
- 0x801F3750 (16, ovl201): garage / mail state block (`Garage3750`); notes suggest one family with 0x8017F6D0 `MailState`.
- 0x801A5F30, 0x801A5F28 (ovl202) and 0x8019D95C, 0x8019D968 (ovl203/204): renderer globals (`RenderGlobals`, `RenderCtx`, `OtVtx`).
- 0x801BCE88 / 0x801BCE18 (ovl201): frame environments `Frame[2]` (0x19458 each).
- Shape-only groups with many sources: 0x68 `LightSet` (11, ovl202-204), `PFT3` / `PFT4` (libgpu `POLY_FT3` / `POLY_FT4` under local names, 21), `DR_MODE` (16).

## Survey tables

Generated by `structsurvey.py table`. "(ptr)" means every source declares a pointer at that
address, so the layout is of the record it points to. "most-named fields" lists the offsets
that the most sources name, with the most common name. "shared type" is the
`include/ac1/symbols.tsv` entry at that exact address. The second table lists layouts that
recur by shape (offsets and widths, names ignored) in two or more sources and are not bound
to an address.

| address | files | units | local names | sizes | most-named fields | shared type |
| --- | --: | --- | --- | --- | --- | --- |
| 0x1F800000 | 81 | ovl202 ovl203 ovl204 | Spad x8, LSpad x8, CScr x4, SprScr x3, ... | 0xC 0x18 0x24 0x2C ... | +0 ot, +4 prim, +8 end, +10 rec, +1C ot, +20 shift4 |  |
| 0x80039D18 (ptr) | 44 | ovl203 ovl204 | Pilot x11, Side3 x6, Player x5, VRec4 x2, ... | 0x12 0x38 0x204 0x224 ... | +10 flags, +80 target, +220 yaw, +254 flags, +394 body, +398 cam | `g_player` VsPlayer * |
| 0x801EF6C8 | 42 | ovl201 | GfxBuf x25, UiGfx x10, Gpu x3, GfxCur, ... | 0x8 | +0 idx, +4 prim |  |
| 0x801A2550 | 35 | ovl201 | PadState x25, PadRepeat x8, PadSt, FePad | 0x20 0x28 | +0 used, +4 delay, +6 delay2, +18 press, +1C held, +24 speed | `g_fe_pad` PadState |
| 0x80198B08 | 26 | ovl202 fdat000 fdat004 fdat006 fdat008 fdat012 fdat014 fdat0 | Slot34 x3, Obj34 x2, Zone x2, Func801C6B30Node, ... | 0x34 0x6A04 | +0 pos, +0 x, +2 y, +4 z, +6 kind, +1C flags | `g_map_objs` MapObj[512] |
| 0x8019FAB8 | 19 | ovl202 fdat000 fdat004 fdat008 fdat012 fdat016 fdat018 fdat0 | Slot x6, World x4, Func801C5428Block, Func801C662CWorld, ... | 0x2C 0x4300 0x6310 0x6324 | +0 slots, +0 st, +1 idx, +A kind, +2C00 ents, +630C frame | `g_spawn_tmpl` SpawnTmpl[256] |
| 0x801D0B68 | 19 | ovl202 fdat000 fdat002 fdat006 fdat028 fdat040 fdat056 fdat0 | World x4, Ws x2, Func801C5314World, Func801C53D4World, ... | 0x40 | +6 id, +26 n_parts, +28 ids, +2C on_hit, +30 flags, +38 stats | `g_world_structs` WorldStruct[] |
| 0x80040FA0 | 17 | ovl202 ovl203 ovl204 fdat096 | PadRec x3, VRec4 x2, PilotBlk x2, Rec, ... | 0x274 0x27C 0x288 0x390 ... | +0 w, +0 now, +8 x, +C xold, +10 flags, +394 ent | `g_players` VsPlayer[2] |
| 0x8017E500 (ptr) | 17 | ovl201 | TmdPktF3 x5, TmdPktG3 x3, TmdPktFT3 x2, TmdPktG4 x2, ... | 0x10 0x14 0x18 0x1C ... | +0 type, +2 count, +8 n0, +A v0, +C uv2, +10 n2 |  |
| 0x8019C090 | 16 | ovl201 | ShopState x15, PartList | 0x8C 0x98 0xE8 0xF8 ... | +1 mode, +2 kind, +5A top, +96 start, +11C kinds, +1BC ids |  |
| 0x801A5F30 | 16 | ovl202 | RenderGlobals x10, TexSlots x3, MGlobals x2, MdlCur | 0x8 0x618 0xB28 0x7738 ... | +0 model, +4 tbl, +614 verts, +7734 fog, +774C cam, +777C cam2 |  |
| 0x801F3750 | 16 | ovl201 | Garage3750 x11, Blk120, State3750, Garage, ... | 0x2C 0x78 0x7C 0x1C4 ... | +0 w, +2 rank, +38 flags, +79 cursor, +84 order, +1944 list |  |
| 0x8019D95C | 15 | ovl203 ovl204 | RenderCtx x8, DrawWork x2, V63Ctx x2, V67Ctx x2, ... | 0x4 0x678 0x684 0x72C8 ... | +0 buf, +0 ctx, +4 ot, +C model, +674 verts, +674 vtx |  |
| 0x80039C78 | 14 | ovl202 ovl204 fdat002 fdat004 fdat010 fdat012 fdat016 fdat01 | Bytes8 x2, Func801C5D8CBytes8, Bytes8_801C5A90, Func801C5190Bytes8, ... | 0x8 |  |  |
| 0x8019D968 | 14 | ovl203 ovl204 | RenderGlobals x11, V67Globals x2, V63Globals | 0x8 0x618 0x66C 0x72EC ... | +0 model, +4 tbl, +668 verts, +6584 light, +729C cam, +72CC cam2 |  |
| 0x801A6A48 | 13 | ovl202 ovl204 | ScrVtx x5, ClipVtx x4, ScrBuf x2, CVtxBuf, ... | 0x10 0x6C20 0x6C24 | +0 sxy, +4 otz, +4 otz, +6 clip, +7 near, +C dp |  |
| 0x801BCE88 | 13 | ovl201 | UiViewSet x7, DrawSlot x5, FrameEnvs | 0x64 0x19458 | +0 view, +0 env, +0 slot, +3E8 rest |  |
| 0x80031A64 | 12 | ovl201 | Rgb3 x10, Raw48 x2 | 0x3 0x30 | +0 r, +1 g |  |
| 0x80031A94 | 12 | ovl201 | Blk160 x4, Blob160 x3, Blk0xA0 x2, Raw160 x2, ... | 0xA0 |  |  |
| 0x80039D30 (ptr) | 12 | ovl203 ovl204 | Pilot x4, PadBlk x2, VRec4, PilotBlk, ... | 0x10 0x12 0x258 0x398 ... | +0 w, +10 flags, +24E blink1, +250 blink2, +254 flags254, +394 body | `g_vs_sides` VsPlayer *[2] |
| 0x801A5F28 | 12 | ovl202 ovl203 ovl204 | OtVtx x5, OtCVtx x3, OtView x2, OtBase, ... | 0x4 0x680 0xB30 0x7774 ... | +0 ot, +670 vtx, +B20 vtx, +7754 cam, +78A4 view |  |
| 0x8017E5D8 | 11 | ovl201 | EmbRgb x6, CVECTOR x3, Rgba x2 | 0x4 | +0 r, +1 g |  |
| 0x80180860 | 11 | ovl201 | LetterCtl x9, MailView x2 | 0x4 0x26 0x40 | +0 work, +1 row, +24 top, +3C items |  |
| 0x8017F6D0 | 10 | ovl201 | MailState x9, Blob800960C0 | 0x40 0xC2 0xC4 0x110 ... | +0 flags, +40 cond, +C0 count, +C1 mode, +D0 order, +1190 view |  |
| 0x801A26B8 | 10 | ovl202 fdat000 fdat004 fdat022 fdat098 | Func801C62A8Obj, PlView, Func801C60B0Obj, Ent, ... | 0x2C 0x38 0x170 | +8 pos, +8 x, +A y, +C z, +12 yaw, +2A h | `g_ac_entities` AcEntity[16] |
| 0x80039D34 (ptr) | 8 | ovl203 ovl204 | PadBlk x2, VRec4, PilotBlk, Pilot, ... | 0x10 0x258 0x398 0x39C | +0 w, +10 flags, +19 cur, +1E hp, +250 blink2, +394 ent |  |
| 0x8017E508 (ptr) | 8 | ovl201 | TmdPkt x8 | 0x10 0x14 0x18 0x1C ... | +0 type, +2 count, +A v0, +E v2, +12 v0, +16 v0 |  |
| 0x8019FB68 | 8 | fdat002 fdat018 fdat056 fdat064 fdat082 fdat094 fdat096 fdat | Slot x6, Slot2C x2 | 0x2C | +0 st, +1 idx, +A kind, +14 flags |  |
| 0x801A5F24 | 8 | ovl202 | DrawWork x8 | 0xB34 | +0 ctx, +8 model, +C model, +B24 vtx |  |
| 0x801E8B74 | 8 | ovl202 | HitRec x6, NearEnt, HitEntry | 0x1C | +0 poly, +4 ws, +8 mesh, +E dist, +14 pt |  |
| 0x80039D20 | 7 | ovl201 | Blk7 x5, Raw7 x2 | 0x7 |  |  |
| 0x80048630 | 7 | ovl201 | Blk48 x4, Blk56 x2, PadMap | 0x30 0x38 | +0 w, +0 w |  |
| 0x80198800 | 7 | ovl202 fdat000 fdat008 fdat014 fdat066 | Func801C6D10Kind, Func801C6ED8Kind, Kind, Func801C830CKind, ... | 0x18 | +E id |  |
| 0x801ADCD8 | 7 | ovl204 | RenderGlobals x7 | 0x7314 0x7334 0x7414 0x7448 | +0 model, +4 tbl, +6584 light, +7284 fog, +72C4 cam, +72F4 cam2 |  |
| 0x801EF6D0 | 7 | ovl201 | CardDir x6, CardInfo | 0x88C | +0 used_blocks, +4 n_files, +8 n_saves, +C files, +44C saves |  |
| 0x80031BD4 | 6 | ovl201 | Blk36 x3, Raw36 x2, AcNames | 0x24 | +0 s |  |
| 0x80042AA8 | 6 | ovl201 | Blob160 x3, Blk160 x2, AcData | 0xA0 |  |  |
| 0x801C40F0 | 6 | ovl203 | WStruct x4, V718Obj, V71DObj | 0x40 | +18 box, +2C car, +30 light |  |
| 0x801D4488 | 6 | ovl204 fdat162 | WStruct x4, World, Cell4 | 0x40 | +10 cx, +12 cy, +14 cz, +26 n, +30 light, +30 flags |  |
| 0x801ED7A0 | 6 | ovl202 | ScrVM x6 | 0xA8 0xB4 | +0 th, +A0 cur, +A4 last, +B0 blob |  |
| 0x80040E38 | 5 | ovl201 | Blk7 x5 | 0x7 |  |  |
| 0x80040FA8 | 5 | ovl202 | PauseCtl, Func80079370Pad, PadCtl7D, PadCtl80, ... | 0x1FC 0x250 0x2A0 | +0 held, +18 flags, +288 key_fwd, +28A key_back, +28C key_a, +28E key_b |  |
| 0x80040FEC (ptr) | 5 | ovl202 | SndCtx x3, Rec20, Rec20b | 0x2 0x5 0x12 0x14 | +1 snd, +11 id |  |
| 0x80041BD0 | 5 | exe ovl202 | SndBank x2, Ent8, Func8004CF0CEntB, SndVab | 0x8 0x4C0 | +0 v, +0 vabs, +4B6 defs |  |
| 0x800B7E48 | 5 | ovl201 | EmbRgb x3, CVECTOR x2 | 0x4 | +0 r, +1 g |  |
| 0x800BA134 | 5 | ovl201 | Part2C x3, Part44, PartRec44 | 0x2C | +0 name, +0 name, +24 price, +28 weight, +2A s2a | `g_parts_arm_weapon` u8[] |
| 0x8017F584 | 5 | ovl201 | RankEnt x5 | 0x10 | +0 name, +4 sub, +8 kind, +8 kind, +C points |  |
| 0x8019B550 | 5 | ovl203 | VsEnt, V718Ent, V71DEnt, V72Ent, ... | 0x170 | +8 x, +8 pos, +A y, +C z, +28 r, +2A h |  |
| 0x8019DF80 | 5 | ovl203 | Rec28 x5 | 0x28 | +0 state |  |
| 0x801A6548 | 5 | ovl202 | Rec28 x5 | 0x28 | +0 state |  |
| 0x801ADCCC | 5 | ovl204 | DrawBlk x3, DrawWork x2 | 0x684 0x72F0 0x7434 0x7440 | +0 buf, +0 ctx, +4 ot, +C model, +674 vtx, +72D0 rot |  |
| 0x801BCEEC | 5 | ovl201 | Screen x3, Func8006C554Screen, DRAWENV | 0x5C 0x19458 | +0 x, +0 clip, +2 y, +8 w, +8 ofs, +A h |  |
| 0x801D62E0 | 5 | ovl201 | DrawSlot x5 | 0x64 | +0 env |  |
| 0x80031B34 | 4 | ovl201 | Blk0xA0 x2, Raw160 x2 | 0xA0 |  |  |
| 0x80040E60 | 4 | ovl201 | Blk160 x4 | 0xA0 |  |  |
| 0x80041B10 | 4 | ovl201 | Blk120 x3, Prog | 0x78 | +0 w, +0 id, +2 rank, +30 n30, +34 n34, +38 flags |  |
| 0x80041DE0 | 4 | exe ovl202 | SndChan x3, Ent10 | 0xA | +0 busy, +2 voice, +3 vol, +4 note, +5 fine, +6 id |  |
| 0x80049F90 | 4 | exe ovl201 ovl203 | LinkState x2, SysCtl, LinkIo | 0x2FC 0x310 | +0 ev, +4 ev2, +2F4 fd_r, +2F8 fd_w, +2FC rx, +308 tx |  |
| 0x8004AC60 | 4 | ovl201 | Rgb3 x4 | 0x3 | +0 r, +1 g |  |
| 0x80194A70 | 4 | ovl203 | Ovl203Obj x3, WObj | 0x34 | +0 pos, +0 x, +2 y, +4 z, +6 type, +8 rot |  |
| 0x801A5F34 (ptr) | 4 | ovl202 | ModelHdr x2, AnimModel, RecHdr | 0xC 0x10 0x1C 0x28 | +5 nparts, +8 off, +C verts, +10 anims, +14 poses, +18 anims2 |  |
| 0x801AB748 | 4 | ovl204 | VsEnt x2, Func80073EC8Obj, Obj | 0x164 0x170 | +8 x, +E model, +60 obj, +148 fire_flags, +15C flags, +160 hp |  |
| 0x801ADCC4 | 4 | ovl204 | Gfx4State x3, FrameG | 0x4 0xC 0x72D8 | +0 frame, +4 heap, +8 buf, +C ot, +72C4 ws, +72C8 view |  |
| 0x801AE82C | 4 | ovl202 | SphereSet x2, Pool300, PLightSet | 0x308 | +0 s, +300 cur, +304 n |  |
| 0x1F800200 | 3 | ovl202 ovl203 ovl204 | Func80062BD8Scratch x2, V62020Scratch | 0x20 | +0 v0, +8 v8 |  |
| 0x80040FB8 | 3 | ovl202 | AimState x2, AimCfg | 0x30 0x2FE | +0 sel, +20 lo, +28 hi, +2C2 tab |  |
| 0x800411F4 | 3 | ovl201 ovl204 | Sys411F4, Rec411F4, Flags32 | 0x4 0x5 0x1E | +0 w, +4 sel, +1C radar |  |
| 0x800419B8 | 3 | ovl201 | Blk36 x3 | 0x24 |  |  |
| 0x80048538 | 3 | ovl201 | Blk48 x3 | 0x30 | +0 w |  |
| 0x80049E70 | 3 | ovl201 | Blk64 x2, Money | 0x40 | +0 w, +0 total, +4 x |  |
| 0x80049EC8 | 3 | ovl201 | Raw196 x2, Blob800960C0 | 0xC4 |  |  |
| 0x80049FE0 | 3 | exe ovl201 | CdQueue x2, Func800155F0Pool | 0x288 | +0 req, +280 wr, +284 rd |  |
| 0x8004A2A4 | 3 | exe | FileEnt, Func80015D80Ent, Func80016678Mxt | 0xC 0xF4 | +0 tab, +0 tab, +4 pos, +F0 heap |  |
| 0x8008ED68 | 3 | ovl202 | Glyph, HudShape, SprDef | 0xC | +0 spr, +8 col |  |
| 0x8008FC48 | 3 | ovl202 | SpreadDef x2, WDef40 | 0x28 0x34 0x38 | +0 speed, +2 jitter, +4 ang1, +6 ang0, +8 arc, +A sfx |  |
| 0x8008FEE8 | 3 | ovl202 | MissileDef24 x2, WDef36 | 0x24 | +0 speed, +2 size, +6 model, +8 sfx, +A burst, +B spread |  |
| 0x80090928 | 3 | ovl202 | Col4 x2, CVec | 0x4 | +0 r, +1 g |  |
| 0x800B9DF4 | 3 | ovl201 | Part34, PartArms, PartRec52 | 0x34 | +0 name, +0 name, +24 price, +28 weight, +2A s2a, +2C s2c | `g_parts_arms` u8[] |
| 0x800BA3F4 | 3 | ovl201 | Part34, PartArms, PartRec52 | 0x34 | +0 name, +0 name, +24 price, +28 weight, +2A s2a, +2C s2c | `g_parts_back` u8[] |
| 0x800BAADC | 3 | ovl201 | Part34, PartArms, PartRec52 | 0x34 | +0 name, +0 name, +24 price, +28 weight, +2A s2a, +2C s2c | `g_parts_blade` u8[] |
| 0x800BABAC | 3 | ovl201 | Part30, Part48, PartRec48 | 0x30 | +0 name, +0 name, +24 price, +28 weight, +2A s2a, +2C s2c | `g_parts_booster` u8[] |
| 0x800BACCC | 3 | ovl201 | Part60, Part96, PartRec96 | 0x60 | +0 name, +0 name, +24 price, +28 weight, +2A s2a, +2C s2c | `g_parts_fcs` u8[] |
| 0x800BAFCC | 3 | ovl201 | Part30, Part48, PartRec48 | 0x30 | +0 name, +0 name, +24 price, +28 weight, +2A s2a, +2C s2c | `g_parts_gen` u8[] |
| 0x800BCAC0 | 3 | ovl201 | FlySet, PartSet, ModelGroup | 0x10 | +0 lims |  |
| 0x8019B484 (ptr) | 3 | ovl203 | VScene4, ModeDef, ScriptHost | 0x4 0xC 0x18 | +0 frame, +8 fn8, +14 start |  |
| 0x8019B48C | 3 | ovl203 | VsState4, LinkErr, Flags32 | 0x4 0x8 0xC | +0 flags, +4 phase, +6 elapsed, +8 clock |  |
| 0x8019B4F8 | 3 | ovl203 fdat182 | Slot, ModeWork, Rec2C | 0x2C 0x338 | +0 kind, +0 slot, +1 idx, +4 v, +58 ent |  |
| 0x8019FC44 | 3 | fdat004 fdat034 fdat096 | Slot x2, Slot2C | 0x2C | +0 state, +A link |  |
| 0x801AB684 | 3 | ovl204 | VsState4, MFlags, Flags32 | 0x4 0xC | +0 flags, +4 phase, +6 elapsed, +8 clock |  |
| 0x801AC860 | 3 | ovl202 | VisCam, DWorld, FlareWorld | 0xE3C 0x1F98 | +0 mask, +0 vis, +104 lights, +E08 camobj, +E1C cam, +F98 flares |  |
| 0x801B4104 | 3 | ovl204 | Flags32 x2, Scene | 0x4 0xEB8 | +0 w, +E98 m |  |
| 0x801B4158 | 3 | ovl204 | SceneG, BbWorld, ScrVis | 0xE34 0xE64 0x1FF4 | +0 vis, +104 light, +E30 cur, +E44 cam, +E64 camt, +FF4 obj |  |
| 0x801BC450 | 3 | ovl201 | SndCtl x3 | 0x24 | +0 vab, +4 seq, +20 cur |  |
| 0x801F5094 (ptr) | 3 | ovl201 | ShopListHdr x3 | 0x26 | +0 count, +1 rows, +24 top |  |
| 0x800315B4 | 2 | ovl202 ovl204 | MATRIX x2 | 0x20 | +0 m, +14 t |  |
| 0x80031A58 | 2 | ovl201 | Raw12 x2 | 0xC |  | `g_ac_parts` u8[12] |
| 0x80039D0A | 2 | ovl203 ovl204 | VPair4 x2 | 0x4 |  |  |
| 0x80040E3F | 2 | ovl201 | Blk7 x2 | 0x7 |  |  |
| 0x80041020 | 2 | ovl202 | TgtState, TSlot | 0x30 0x184 | +0 target, +4 list, +16 flags, +18 aim | `g_target_head` TargetSlot * |
| 0x80041024 | 2 | ovl202 | TgtWork, TSlot | 0x30 0x294 | +0 slot, +0 target, +16 flags, +18 aim, +254 on, +256 mode | `g_target_list` TargetSlot[8] |
| 0x800411C0 | 2 | ovl202 | Func8005156CView, Func80079370Scroll | 0x2 0x6 | +0 pitch, +2 vel, +4 timer |  |
| 0x8004127A | 2 | ovl202 | Slot3C, AmmoSlot | 0x3C | +E max, +10 left, +12 price, +24 flags |  |
| 0x80042A86 | 2 | ovl202 | SndSlot x2 | 0x8 | +0 id, +2 sub |  |
| 0x800485A0 | 2 | ovl201 | Blk96 x2 | 0x60 | +0 w |  |
| 0x8008A884 | 2 | ovl204 | Glyph, SprRgb | 0xC | +0 clut, +0 s, +2 tpage, +4 u, +5 v, +6 w |  |
| 0x8008DCA0 | 2 | ovl202 | Col4 x2 | 0x4 |  |  |
| 0x8008E530 | 2 | ovl202 fdat000 | Func801C857CSpec, WDef | 0x1C | +8 range, +18 tag | `g_weapon_defs` WeaponDef[20] |
| 0x8008E610 | 2 | ovl202 fdat000 | Func801C88ECSpread, WpnRec8 | 0x8 | +2 s2, +3 s3, +4 spread_x, +6 spread_y | `g_weapon_spreads` WeaponSpread[9] |
| 0x8008E658 | 2 | ovl202 fdat000 | Func801C7DA8Spec, Spec | 0x6 | +0 range, +2 kick, +4 trig, +5 cool | `g_weapon_specs` WeaponSpec[7] |
| 0x8008EC88 | 2 | ovl202 | Func80087070Obj, Func80087798Ent | 0xD0 | +10 state, +14 fn, +18 v18, +18 pos, +20 v20, +B8 pos | `g_proj_none` Projectile |
| 0x8008FB48 | 2 | ovl202 | Rgba x2 | 0x4 | +0 r, +1 g |  |
| 0x8008FB58 | 2 | ovl202 | WDef24, ShellDef | 0x18 | +0 range, +2 jitter, +8 arc, +A sfx, +C col |  |
| 0x8008FEC8 | 2 | ovl202 | FxPair x2 | 0x4 |  |  |
| 0x80090354 | 2 | ovl202 | MissileDef x2 | 0xE | +4 glow, +9 col |  |
| 0x80090A28 | 2 | ovl202 | Col4 x2 | 0x4 |  |  |
| 0x800B6E4C | 2 | ovl201 | Entry0C x2 | 0xC | +0 raw |  |
| 0x800B91F4 | 2 | ovl201 | PartHead, PartRec60 | 0x3C | +0 name, +0 name, +24 price, +28 weight | `g_parts_head` u8[] |
| 0x800B944C | 2 | ovl201 | PartCore, PartRec56 | 0x38 | +0 name, +0 name, +24 price, +28 weight, +32 maxload | `g_parts_core` u8[] |
| 0x800B94F4 | 2 | ovl201 | PartLegs, PartRec72 | 0x48 | +0 name, +0 name, +24 price, +28 weight, +32 maxload | `g_parts_legs` u8[] |
| 0x800BB140 | 2 | ovl201 | OptRow, Func8008F08CRec44 | 0x2C | +0 price |  |
| 0x800BC428 | 2 | ovl201 | Vec3u, SVECTOR | 0x8 | +0 x, +2 y, +4 z |  |
| 0x800BC440 | 2 | ovl201 | CoreJoints, Func80080224Ofs | 0x28 | +0 j |  |
| 0x800BCABC | 2 | ovl201 | PartCat x2 | 0x10 | +0 mask, +4 pose |  |
| 0x800BCAC4 | 2 | ovl201 | PartOfsTbl, Func80080224Set | 0x10 | +0 ofs |  |
| 0x800BDDA0 | 2 | ovl201 | Blk48, PadMap | 0x30 | +0 w |  |
| 0x800BDFC8 | 2 | ovl201 | RankEnt x2 | 0x10 | +0 name, +4 sub, +8 kind, +C points |  |
| 0x8017E618 | 2 | ovl201 | EmbRgb x2 | 0x4 | +0 r, +1 g |  |
| 0x80198808 | 2 | ovl202 | NodeType, Rec24 | 0x18 | +0 h |  |
| 0x8019B4A0 | 2 | ovl203 | PartSlot, Ovl203Actors | 0x2C 0x390 | +0 defs, +B0 ents |  |
| 0x8019D960 | 2 | ovl203 | V62Globals, V63Globals | 0x674 | +0 ot, +670 verts |  |
| 0x8019F308 (ptr) | 2 | ovl202 | HitObj x2 | 0x22 | +1C flags, +1E w, +20 h | `g_map_obj_list` MapObj *[128] |
| 0x8019F524 | 2 | ovl202 | MFlags, VmFlags | 0x4 | +0 flags | `g_mission_flags` u32 |
| 0x8019F538 | 2 | ovl202 | Rec2C, AcWorld | 0x2C 0x4880 | +0 geoms |  |
| 0x8019F564 | 2 | ovl202 | TexTbl x2 | 0x685C | +0 tex, +6858 cur |  |
| 0x801A5DD0 | 2 | ovl202 | HitWatch x2 | 0x8 | +0 flag, +4 hit |  |
| 0x801A60C4 | 2 | ovl203 | FxObj, CProj3 | 0xE0 | +0 x, +0 pos, +2 y, +4 z, +6 free, +8 owner |  |
| 0x801A6134 (ptr) | 2 | ovl202 fdat000 | Func801C7DA8Set, Set | 0x18 | +14 table |  |
| 0x801AB67C (ptr) | 2 | ovl204 | VScene4, Scene4 | 0x4 0x18 | +0 frame, +14 init |  |
| 0x801AC820 | 2 | ovl202 | FxPool, SectVis | 0xE7C 0x2314 | +0 mask, +0 drawn, +40 seen, +E48 cur, +E70 cam, +200C ent |  |
| 0x801AC8A4 | 2 | ovl202 | Rec18, VramSave | 0x18 | +0 used, +1 period, +2 timer, +3 speed, +4 rect, +C ofs |  |
| 0x801AD668 (ptr) | 2 | ovl202 | RoomDef, DCam | 0x38 0x3C | +30 lo, +34 reverb, +38 hi |  |
| 0x801AD66C | 2 | ovl202 | Func80062BD8View, CamPose | 0x10 0x11F0 | +0 pose, +0 pos, +8 rot, +30 v30, +60 mats, +11C0 entry |  |
| 0x801ADA48 | 2 | ovl204 | Gfx4FrameSet x2 | 0x27C | +0 flip |  |
| 0x801AEB3C | 2 | ovl202 | PEnt, CProj3 | 0xE0 | +0 pos, +0 pos, +6 free, +8 owner, +E ws, +10 type | `g_proj_pool` Projectile[256] |
| 0x801B4118 | 2 | ovl204 | FxPool, World4 | 0xE74 0x233C | +0 mask, +0 vis, +40 drawn, +E70 cur, +2034 ent, +2334 cur |  |
| 0x801B4F8C | 2 | ovl204 | SkyPose, Func80062BD8View | 0x10 0x11F0 | +0 pos, +0 pose, +8 rot, +30 v30, +60 mats, +11C0 entry |  |
| 0x801B645C | 2 | ovl204 | FxObj, CProj3 | 0xE0 | +0 x, +0 pos, +2 y, +4 z, +6 free, +8 owner |  |
| 0x801BC454 | 2 | ovl201 | SeqState x2 | 0x1E | +0 seq, +1C cur |  |
| 0x801BCDE8 | 2 | ovl201 | Rgb3 x2 | 0x3 | +0 r, +1 g |  |
| 0x801BCE18 | 2 | ovl201 | Frame x2 | 0x19458 | +0 draw, +5C disp, +70 layer, +458 prim |  |
| 0x801D0B98 | 2 | ovl202 | LightSel x2 | 0x40 | +0 idx |  |
| 0x801D0B9C | 2 | fdat016 | WSf x2 | 0x40 | +0 flags |  |
| 0x801F36D0 (ptr) | 2 | ovl201 | TmdHdr, Tmd | 0x28 | +0 id, +4 flags, +8 nobj, +C obj |  |
| 0x801F5098 | 2 | ovl201 | SaveEntry, SaveSlot | 0x2C | +0 name |  |
| 0x80031764 | 1 | ovl202 | CdaTrackRow | 0x24 | +0 tracks, +20 loc |  |
| 0x80031784 | 1 | exe | SndFile | 0x24 | +0 loc |  |
| 0x80031BE6 | 1 | ovl201 | Name18 | 0x12 |  |  |
| 0x80031BF4 | 1 | ovl201 | SjisCh | 0x2 | +0 hi, +1 lo |  |
| 0x80040850 | 1 | ovl201 | TimDesc | 0x24 |  |  |
| 0x80040FB0 | 1 | ovl202 | HudState | 0x306 | +0 flags, +9 cur, +E gauge, +10 flags2, +216 box1, +226 box2 |  |
| 0x80040FCE | 1 | ovl202 | Marker | 0xA | +0 on, +2 pos |  |
| 0x80040FD8 | 1 | ovl202 | AimBlk | 0x286 | +0 lo, +280 v |  |
| 0x800411A8 | 1 | ovl202 | HoverCfg | 0x10 | +0 sink, +2 floor, +4 accel, +6 max_speed |  |
| 0x800411B0 | 1 | ovl202 | MoveDef | 0x4 | +0 accel, +2 max_speed |  |
| 0x800411C6 | 1 | fdat004 | Func801C6A64Box | 0x10 | +0 lo, +8 hi |  |
| 0x800411D6 | 1 | fdat004 | Func801C6A64Box | 0x10 | +0 lo, +8 hi |  |
| 0x80041228 | 1 | ovl202 | RadarCfg | 0x3A | +4 r, +38 tail |  |
| 0x8004124F | 1 | ovl202 | SoundSet | 0x3 | +0 n, +1 s |  |
| 0x80041258 | 1 | ovl202 | V3 | 0x6 | +0 vx, +2 vy, +4 vz |  |
| 0x80041B90 | 1 | ovl202 | Pair8 | 0x8 |  |  |
| 0x80041BA0 (ptr) | 1 | ovl202 | RoomDef | 0x3C | +30 lo, +34 reverb, +38 hi |  |
| 0x80041BB8 | 1 | exe | SndState | 0xA | +0 vol, +6 seq, +8 flag |  |
| 0x80041BBE | 1 | ovl202 | CdaState | 0x6 | +0 track, +2 playing, +3 flags, +4 state |  |
| 0x80041BC2 | 1 | ovl202 | VolByte | 0x1 |  |  |
| 0x80041BC8 | 1 | ovl202 | RevPreset | 0x7 | +0 type, +1 depth_l, +2 depth_r, +3 vol_l, +4 vol_r, +5 feedback |  |
| 0x80041EC6 | 1 | ovl202 | RevPreset | 0x7 | +0 type, +1 depth_l, +2 depth_r, +3 vol_l, +4 vol_r, +5 feedback |  |
| 0x80042086 | 1 | ovl202 | Func8004CF0CEntA | 0xA | +0 v |  |
| 0x800485B8 | 1 | ovl201 | Pair485 | 0x8 |  |  |
| 0x80048610 | 1 | ovl201 | Result | 0x8 | +0 failed, +4 bonus |  |
| 0x80049FA0 | 1 | exe | LoadingBar | 0x2C | +0 count, +4 prim |  |
| 0x8004A260 | 1 | exe | CdQueue | 0x8 | +0 wr, +4 rd |  |
| 0x8004A264 (ptr) | 1 | exe | CdReq | 0x28 | +0 cmd, +1 status, +2 loc, +6 cur, +C dst, +10 sectors |  |
| 0x8004A284 | 1 | ovl203 | LinkIo | 0x1C | +0 rfd, +8 rx, +14 tx |  |
| 0x8004A288 | 1 | ovl203 | Link3 | 0xC | +0 rfd |  |
| 0x8004AC38 | 1 | ovl201 | AcStats | 0x20 |  |  |
| 0x8004AC3C | 1 | ovl201 | AcTotals | 0xC | +0 ap, +4 weight, +8 price |  |
| 0x8004AC90 | 1 | ovl201 | Rgb3 | 0x3 | +0 r, +1 g |  |
| 0x8004B15C | 1 | ovl203 | V79Trip | 0x6 |  |  |
| 0x8004B164 | 1 | ovl203 | V79Trip | 0x6 |  |  |
| 0x8004B16C | 1 | ovl203 | V79Trip | 0x6 |  |  |
| 0x8004B3F8 | 1 | ovl202 | FlareCol | 0xC |  |  |
| 0x8004B4D8 | 1 | ovl202 | V3 | 0x6 | +0 vx, +2 vy, +4 vz |  |
| 0x8004B4E0 | 1 | ovl202 | V3 | 0x6 | +0 vx, +2 vy, +4 vz |  |
| 0x8004B4E8 | 1 | ovl202 | V3 | 0x6 | +0 vx, +2 vy, +4 vz |  |
| 0x8004B4FC | 1 | ovl201 | RECT16 | 0x8 | +0 x, +2 y, +4 w, +6 h |  |
| 0x8004BDC0 | 1 | ovl201 | CatList | 0x18 | +0 v |  |
| 0x8004BF60 | 1 | ovl201 | Str9 | 0x9 | +0 s |  |
| 0x8004C158 | 1 | ovl202 | Rgba | 0x4 | +0 r, +1 g |  |
| 0x8004C15C | 1 | ovl202 | Rgba | 0x4 | +0 r, +1 g |  |
| 0x8004C160 | 1 | ovl202 | Rgba | 0x4 | +0 r, +1 g |  |
| 0x8004C4F8 | 1 | ovl201 | Palette | 0x30 |  |  |
| 0x8004CD84 | 1 | ovl201 | Text9 | 0x9 |  |  |
| 0x8004D150 | 1 | ovl201 | Name18 | 0x12 |  |  |
| 0x80089484 | 1 | ovl204 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x800894A4 | 1 | ovl204 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x80089628 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x8008963C | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x8008964C | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x8008965C | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x8008966C | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x8008967C | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x8008968C | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x80089690 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x800896C8 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x800896CC | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x800896E0 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x800896E4 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x800896F4 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x800896FC (ptr) | 1 | ovl204 | BbFrame | 0x30 | +0 rgb, +4 uv0, +8 uv1, +C uv2, +E uv3, +10 v |  |
| 0x80089710 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x80089720 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x80089730 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x80089740 | 1 | ovl203 | Pt16 | 0x4 | +0 x, +2 y |  |
| 0x80089980 (ptr) | 1 | ovl203 | SprFrame | 0x30 | +0 rgbc, +4 uv0, +8 uv1, +C uv2, +E uv3, +10 v |  |
| 0x8008A220 | 1 | ovl203 | V7BDef | 0x1C |  |  |
| 0x8008A300 | 1 | ovl203 | WEnt3 | 0x8 |  |  |
| 0x8008A348 | 1 | ovl203 | FSpec3 | 0x6 | +0 range, +2 kick, +4 mode |  |
| 0x8008AA58 | 1 | ovl203 | SprRgb | 0xC | +0 s, +8 rgb |  |
| 0x8008B848 | 1 | ovl203 | V7BTabD | 0x18 | +0 v |  |
| 0x8008B938 | 1 | ovl203 | V7BTabB | 0x28 | +0 v |  |
| 0x8008BBD8 | 1 | ovl203 | V7BTabC | 0x24 | +0 v |  |
| 0x8008C2B8 | 1 | ovl204 | VRec4 | 0x39C | +10 flags, +390 wins, +394 ent |  |
| 0x8008C490 | 1 | ovl203 | VRec4 | 0x39C | +10 flags, +390 wins, +394 ent |  |
| 0x8008C654 | 1 | ovl204 | VRec4 | 0x39C | +10 flags, +390 wins, +394 ent |  |
| 0x8008C884 | 1 | ovl203 | Bytes8 | 0x8 |  |  |
| 0x8008D938 | 1 | ovl202 | HudSpr | 0xC |  |  |
| 0x8008D968 | 1 | ovl202 | HudSpr | 0xC |  |  |
| 0x8008D974 | 1 | ovl202 | HudSpr | 0xC |  |  |
| 0x8008D988 | 1 | ovl202 | HudSpr | 0xC |  |  |
| 0x8008D998 | 1 | ovl202 | HudSpr | 0xC |  |  |
| 0x8008D9A8 | 1 | ovl202 | HudSpr | 0xC |  |  |
| 0x8008D9B8 | 1 | ovl202 | HudSpr | 0xC |  |  |
| 0x8008D9C8 | 1 | ovl202 | HudSpr | 0xC |  |  |
| 0x8008DABC | 1 | ovl202 | Rec10 | 0xA | +8 v |  |
| 0x8008DB84 | 1 | ovl202 | Rec10 | 0xA | +8 v |  |
| 0x8008DB98 | 1 | ovl202 | IconSpr | 0xC |  |  |
| 0x8008DC90 (ptr) | 1 | ovl202 | FlareDef | 0x30 | +0 rgb, +4 uv0, +8 uv1, +C uv2, +E uv3, +10 v |  |
| 0x8008E684 | 1 | ovl202 | Rec20 | 0x14 |  |  |
| 0x8008EFFC | 1 | ovl202 | HudShape | 0xC |  |  |
| 0x8008FC18 | 1 | ovl202 | MortarDef | 0x18 | +0 speed, +4 life, +6 sfx, +8 count, +A jitter, +C spread |  |
| 0x80090164 | 1 | ovl202 | CurveDef | 0x8 | +4 curve |  |
| 0x80090198 | 1 | ovl202 | Def28 | 0x28 | +6 life |  |
| 0x8009019C | 1 | ovl202 | MissileDef | 0x28 | +0 life, +C guided, +E maxspd, +10 turn, +12 cone, +20 fx |  |
| 0x80090364 | 1 | ovl202 | Func80087798Def | 0x30 | +22 p22 |  |
| 0x800906C4 | 1 | ovl202 | WaveDef | 0x8 | +0 life, +2 phase, +4 amp, +6 step |  |
| 0x800906C8 | 1 | ovl202 | Wave8 | 0x8 | +2 step |  |
| 0x80090798 | 1 | ovl202 | Rgba | 0x4 | +0 r, +1 g |  |
| 0x800907A0 | 1 | ovl202 | SprCell | 0xC | +0 spr |  |
| 0x80091228 | 1 | ovl202 | RelocPair | 0x8 |  |  |
| 0x800B1248 | 1 | ovl202 | SrcRec2C | 0x2C |  |  |
| 0x800B6DF8 | 1 | ovl201 | Entry0C | 0xC | +0 raw |  |
| 0x800B70D4 | 1 | ovl201 | Blk0xA0 | 0xA0 |  |  |
| 0x800B7174 | 1 | ovl201 | Blk0xA0 | 0xA0 |  |  |
| 0x800B7224 | 1 | ovl201 | Blk0xA0 | 0xA0 |  |  |
| 0x800B72C4 | 1 | ovl201 | Blk0xA0 | 0xA0 |  |  |
| 0x800B7384 | 1 | ovl201 | AcNames | 0x24 | +0 s |  |
| 0x800B73A8 | 1 | ovl201 | Rgb3 | 0x3 |  |  |
| 0x800B7470 | 1 | ovl201 | GlyphBlock | 0x6 | +0 code, +2 u, +4 v |  |
| 0x800B755C | 1 | ovl201 | BoxRgb | 0x4 | +0 r, +1 g |  |
| 0x800B7570 | 1 | ovl201 | BoxRgb | 0x4 | +0 r, +1 g |  |
| 0x800B7574 | 1 | ovl201 | BoxRgb | 0x4 | +0 r, +1 g |  |
| 0x800B7588 | 1 | ovl201 | Func80060168Tex | 0xC | +2 clut, +8 tpage |  |
| 0x800B75A0 | 1 | ovl201 | RECT16 | 0x8 | +0 x, +2 y, +4 w, +6 h |  |
| 0x800B75B8 | 1 | ovl201 | GlyphPage | 0x6 | +0 first, +2 tpage, +4 clut_y |  |
| 0x800B7E08 | 1 | ovl201 | EmbRgb | 0x4 | +0 r, +1 g |  |
| 0x800B7E54 | 1 | ovl201 | EmbRgb | 0x4 | +0 r, +1 g |  |
| 0x800B7E58 | 1 | ovl201 | EmbRgb | 0x4 | +0 r, +1 g |  |
| 0x800B7EE0 | 1 | ovl201 | EmbSample | 0xC |  |  |
| 0x800B8C74 | 1 | ovl201 | TextDef | 0xC |  |  |
| 0x800B8D78 | 1 | ovl201 | SVECTOR | 0x8 | +0 vx, +2 vy, +4 vz |  |
| 0x800B8E44 | 1 | ovl201 | MATRIX | 0x20 | +0 m, +14 t |  |
| 0x800B8F24 | 1 | ovl201 | P3 | 0x6 | +0 x, +2 y, +4 z |  |
| 0x800B9218 | 1 | ovl201 | Func8008F08CRec60 | 0x3C | +0 value |  |
| 0x800B921C | 1 | ovl201 | Head60 | 0x3C | +0 w |  |
| 0x800B921E | 1 | ovl201 | Row60 | 0x3C | +0 v |  |
| 0x800B9222 | 1 | ovl201 | Row60 | 0x3C | +0 off |  |
| 0x800B9470 | 1 | ovl201 | Func8008F08CRec56 | 0x38 | +0 value |  |
| 0x800B9474 | 1 | ovl201 | Core56 | 0x38 | +0 w, +A load |  |
| 0x800B9476 | 1 | ovl201 | Row56 | 0x38 | +0 v |  |
| 0x800B947A | 1 | ovl201 | Row56 | 0x38 | +0 off |  |
| 0x800B9482 | 1 | ovl201 | Rec56 | 0x38 |  |  |
| 0x800B9518 | 1 | ovl201 | Func8008F08CRec72 | 0x48 | +0 value |  |
| 0x800B951C | 1 | ovl201 | Legs72 | 0x48 | +0 w, +A load |  |
| 0x800B951E | 1 | ovl201 | Row72 | 0x48 | +0 v |  |
| 0x800B9522 | 1 | ovl201 | Row72 | 0x48 | +0 off, +8 stab, +A mob, +12 boost |  |
| 0x800B9E18 | 1 | ovl201 | Func8008F08CRec52 | 0x34 | +0 value |  |
| 0x800B9E1C | 1 | ovl201 | Row52 | 0x34 | +0 w |  |
| 0x800B9E1E | 1 | ovl201 | Row52 | 0x34 | +0 v |  |
| 0x800B9E22 | 1 | ovl201 | Row52 | 0x34 | +0 off |  |
| 0x800BA158 | 1 | ovl201 | Func8008F08CRec44 | 0x2C | +0 value |  |
| 0x800BA15C | 1 | ovl201 | Row44 | 0x2C | +0 w |  |
| 0x800BA15E | 1 | ovl201 | Row44 | 0x2C | +0 v |  |
| 0x800BA418 | 1 | ovl201 | Func8008F08CRec52 | 0x34 | +0 value |  |
| 0x800BA41C | 1 | ovl201 | Row52 | 0x34 | +0 w |  |
| 0x800BA41E | 1 | ovl201 | Row52 | 0x34 | +0 v |  |
| 0x800BAB00 | 1 | ovl201 | Func8008F08CRec52 | 0x34 | +0 value |  |
| 0x800BAB04 | 1 | ovl201 | Row52 | 0x34 | +0 w |  |
| 0x800BAB06 | 1 | ovl201 | Row52 | 0x34 | +0 v |  |
| 0x800BAB0C | 1 | ovl201 | Row52w | 0x34 | +0 weight |  |
| 0x800BABD0 | 1 | ovl201 | Func8008F08CRec48 | 0x30 | +0 value |  |
| 0x800BABD4 | 1 | ovl201 | Row48 | 0x30 | +0 w |  |
| 0x800BABD6 | 1 | ovl201 | Row48 | 0x30 | +0 v |  |
| 0x800BABD8 | 1 | ovl201 | Row48 | 0x30 | +0 speed |  |
| 0x800BACF0 | 1 | ovl201 | Func8008F08CRec96 | 0x60 | +0 value |  |
| 0x800BACF4 | 1 | ovl201 | Row96 | 0x60 | +0 w |  |
| 0x800BACF6 | 1 | ovl201 | Row96 | 0x60 | +0 v |  |
| 0x800BACF8 | 1 | ovl201 | Row96 | 0x60 | +4 sub |  |
| 0x800BAFF0 | 1 | ovl201 | Func8008F08CRec48 | 0x30 | +0 value |  |
| 0x800BAFF4 | 1 | ovl201 | Row48 | 0x30 | +0 w |  |
| 0x800BAFF6 | 1 | ovl201 | Row48 | 0x30 | +0 v |  |
| 0x800BB11C | 1 | ovl201 | PartRec44 | 0x2C | +0 name, +24 price |  |
| 0x800BB144 | 1 | ovl201 | Rec44 | 0x2C |  |  |
| 0x800BB314 | 1 | ovl201 | Row24 | 0x18 | +0 weight |  |
| 0x800BB3D4 | 1 | ovl201 | Row24 | 0x18 | +0 weight |  |
| 0x800BB554 | 1 | ovl201 | Row24 | 0x18 | +0 weight |  |
| 0x800BC638 | 1 | ovl201 | Vec3u | 0x8 | +0 x, +2 y, +4 z |  |
| 0x800BC6B8 | 1 | ovl201 | Vec3u | 0x8 | +0 x, +2 y, +4 z |  |
| 0x800BC968 | 1 | ovl201 | Vec3u | 0x8 | +0 x, +2 y, +4 z |  |
| 0x800BCAB8 | 1 | ovl201 | PartTab | 0x10 | +2 base, +4 list |  |
| 0x800BCB50 | 1 | ovl201 | ModelTab | 0x4 | +0 ent |  |
| 0x800BDC68 | 1 | ovl201 | MovieEnt | 0x8 | +0 file, +4 frames |  |
| 0x8017E474 | 1 | ovl201 | SaveRowText | 0x1B | +0 no |  |
| 0x8017F3E8 | 1 | ovl201 | DISPENV_ | 0x14 |  |  |
| 0x8017F3FC | 1 | ovl201 | Strm | 0x38 | +8 dcur, +C buf, +14 cur, +18 ent, +28 sel, +2C rect |  |
| 0x8017F408 | 1 | ovl201 | MovBufs | 0xC | +0 buf, +8 cur |  |
| 0x8017F414 | 1 | ovl201 | MovSeg | 0x8 | +0 x, +2 y, +4 len |  |
| 0x8017F710 | 1 | ovl201 | Func80096550Cond | 0x2 | +0 type, +1 arg |  |
| 0x8017F794 | 1 | ovl201 | MailState | 0x3 |  |  |
| 0x8018E468 | 1 | ovl201 | BackCol | 0x3 | +0 r, +1 g |  |
| 0x80194738 | 1 | ovl203 | Cam3 | 0x30 | +2C ox, +2E oy |  |
| 0x801987D0 | 1 | ovl202 | CamState | 0x34 | +0 mode, +2C vec |  |
| 0x801987D8 | 1 | fdat004 | Func801C6528Cam | 0x1C | +0 flags, +14 pos |  |
| 0x801987F8 | 1 | ovl202 | WObjDef | 0x18 | +16 kind |  |
| 0x80198C14 | 1 | fdat074 | V8 | 0x8 | +0 x, +2 y, +4 z, +6 w |  |
| 0x8019B488 | 1 | ovl203 | MdlBuf | 0x4 | +0 cur |  |
| 0x8019B88C | 1 | ovl203 | Rec170 | 0x170 |  |  |
| 0x8019DB6C (ptr) | 1 | ovl203 | FSet3 | 0x18 | +14 off |  |
| 0x8019F51C (ptr) | 1 | ovl202 | Objective | 0x10 | +C report | `g_objective_obj` void * |
| 0x8019FBEC | 1 | fdat004 | Slot2C | 0x2C | +0 state, +A link |  |
| 0x8019FC18 | 1 | fdat004 | Slot2C | 0x2C | +0 state, +A link |  |
| 0x8019FC70 | 1 | fdat004 | Slot2C | 0x2C | +0 state, +A link |  |
| 0x8019FC9C | 1 | fdat056 | Slot | 0x2C | +0 st |  |
| 0x8019FE28 | 1 | fdat002 | Slot2C | 0x2C | +0 state, +1 idx, +A link |  |
| 0x801A2310 (ptr) | 1 | ovl201 | UiDialogBox | 0xC | +0 x, +1 y, +8 tpage |  |
| 0x801A26C0 | 1 | fdat040 | P | 0x24 | +0 x, +2 y, +4 z, +22 h |  |
| 0x801A2784 | 1 | fdat004 | Bytes8 | 0x8 |  |  |
| 0x801A2828 | 1 | fdat034 | Ent | 0x170 |  |  |
| 0x801A3D94 | 1 | ovl203 | Flags32 | 0x4 | +0 w |  |
| 0x801A3DE8 | 1 | ovl203 | SceneG | 0xE3C | +0 vis, +104 light, +E08 cur, +E1C cam |  |
| 0x801A3E28 | 1 | ovl203 | Screen | 0xDFC | +0 rgb, +DDC cam |  |
| 0x801A4BF4 | 1 | ovl203 | V62020View | 0x11F0 | +0 pose, +30 v30, +60 mats, +11C0 entry |  |
| 0x801A4C68 | 1 | ovl204 | WObj | 0x34 | +0 x, +2 y, +4 z, +6 type, +8 rot, +10 obj |  |
| 0x801A4D54 | 1 | ovl203 | RotPart | 0x14 | +0 w |  |
| 0x801A5E1C | 1 | ovl202 | EnvBuf | 0x80 | +4 env, +1C isbg |  |
| 0x801AB6F0 | 1 | ovl204 | VsEnts | 0x338 | +58 ent |  |
| 0x801AC8A0 | 1 | ovl202 | ScreenFx | 0xDFC | +0 col, +DDC cam |  |
| 0x801AC964 | 1 | ovl202 | DstRec68 | 0x68 |  |  |
| 0x801AD82C | 1 | ovl202 | Rec16 | 0x10 | +0 v |  |
| 0x801ADA4C | 1 | ovl204 | FrameBuf | 0x13C | +C0 env |  |
| 0x801ADCDC (ptr) | 1 | ovl204 | ModelHdr | 0xC | +8 off |  |
| 0x801AE340 | 1 | ovl204 | PVtx | 0x10 | +0 sxy, +4 otz, +6 clip, +C p |  |
| 0x801B4108 | 1 | ovl204 | Gfx4Counters | 0xC | +0 rcnt |  |
| 0x801B4F60 | 1 | ovl204 | ViewSet | 0x28 | +0 ws0, +4 p0, +14 ws1, +18 p1 |  |
| 0x801B4F9C | 1 | ovl204 | RotPart | 0x14 | +0 w |  |
| 0x801B50EC | 1 | ovl204 | RotPart | 0x14 | +0 w |  |
| 0x801B80C8 | 1 | ovl203 | ModeDef | 0x18 | +14 start |  |
| 0x801BC4F8 | 1 | ovl201 | AsmOwner | 0x98 | +0 hdr, +88 arms, +8C sel, +94 extra | `g_ui_none` UiObj |
| 0x801BC5E8 | 1 | ovl201 | ActiveList | 0x7FC | +0 end, +4 active | `g_ui_active_end` UiObj ** |
| 0x801BD018 | 1 | ovl201 | DRAWENV | 0x5C | +0 clip, +8 ofs, +C rest |  |
| 0x801C4C9C | 1 | fdat044 | CVECTOR | 0x4 | +0 r, +1 g |  |
| 0x801C4CA0 | 1 | fdat092 | Blk6 | 0x6 |  |  |
| 0x801C4D48 | 1 | fdat074 | Blk6 | 0x6 |  |  |
| 0x801C6DD4 | 1 | fdat002 | Func801C6064Turn | 0x6 | +4 speed |  |
| 0x801C6DDC | 1 | fdat002 | Func801C6504Bits | 0xC | +0 lo, +4 angle, +8 hi |  |
| 0x801C6DE0 | 1 | fdat002 | Func801C6504Bits | 0xC | +0 lo, +4 angle, +8 hi |  |
| 0x801C70E8 | 1 | fdat004 | Rec | 0x10 | +8 dur, +A mode3, +B sfx, +F mode2 |  |
| 0x801C7538 | 1 | fdat022 | Func801C60B0Key | 0xE | +2 dy, +4 spd, +8 dur, +A event, +B vol, +C pitch |  |
| 0x801C7540 | 1 | fdat022 | Func801C66C4Ent | 0xE | +2 kind |  |
| 0x801C8460 | 1 | ovl204 | Scene4 | 0x18 | +14 init |  |
| 0x801CDAF4 | 1 | fdat096 | BF | 0xC | +0 lo, +8 hi |  |
| 0x801CDAF8 | 1 | fdat096 | BF | 0xC | +0 lo, +8 hi |  |
| 0x801D0B60 (ptr) | 1 | ovl202 | VmThr | 0x18 | +4 obj, +C kind, +14 mem |  |
| 0x801D23D4 (ptr) | 1 | fdat034 | Vt | 0x8 | +4 fn |  |
| 0x801D3214 (ptr) | 1 | fdat036 | Obj | 0x11 |  |  |
| 0x801D3220 (ptr) | 1 | fdat036 | Obj | 0x11 |  |  |
| 0x801D3260 (ptr) | 1 | fdat036 | Obj | 0x11 |  |  |
| 0x801D3314 (ptr) | 1 | fdat096 | Obj | 0x11 |  |  |
| 0x801D3614 (ptr) | 1 | fdat096 | Obj | 0x11 |  |  |
| 0x801D3A20 (ptr) | 1 | fdat038 | Obj | 0x7 |  |  |
| 0x801D3AE0 (ptr) | 1 | fdat038 | Obj | 0x7 |  |  |
| 0x801D3C20 (ptr) | 1 | fdat038 | Obj | 0x7 |  |  |
| 0x801D3C60 (ptr) | 1 | fdat038 | Obj | 0x7 |  |  |
| 0x801D6344 | 1 | ovl201 | DRAWENV | 0x5C | +0 clip, +8 ofs, +C rest |  |
| 0x801D6470 | 1 | ovl201 | DRAWENV | 0x5C | +0 clip, +8 ofs, +C rest |  |
| 0x801E8B68 | 1 | ovl202 | Heap | 0x4 | +0 cur |  |
| 0x801E8B70 | 1 | ovl202 | Buckets | 0x4C20 | +0 n, +1C20 tab |  |
| 0x801ED790 | 1 | ovl202 | MapCam | 0xC | +0 x, +2 y, +4 z, +8 pitch, +A yaw |  |
| 0x801ED844 (ptr) | 1 | ovl202 | MsgWin | 0x14 | +C mode, +10 text |  |
| 0x801F37C8 | 1 | ovl201 | ListState | 0x3 | +1 cursor, +2 scroll |  |
| 0x801F3814 | 1 | ovl201 | ListEnt | 0x4 | +0 lo, +1 hi, +2 kind |  |

| layout | files | units | local names | named fields |
| --- | --: | --- | --- | --- |
| struct 0x4 | 39 | exe ovl201 ovl202 ovl203 ovl204 | Rgbc x21, Rgba x7, Col3 x3, CdlLOC x2, ... | minute, second, sector, track |
| struct 0x8 | 20 | exe ovl201 ovl202 ovl203 ovl204 | RECT16 x4, RECT x2, StrmEnt x2, AnimKey x2, ... | q, q1, half, r |
| struct 0x6 | 19 | ovl202 ovl203 fdat000 fdat002 fdat010 fdat016 | Vec3s x11, Func801C5834Vec, Func801C674CVec, Func801C63F8Stats, ... | x, y, z |
| struct 0xC | 17 | ovl201 ovl202 ovl204 | DR_MODE x8, DrMode x8, BoxTwin, BoxArea, ... | tag, code |
| struct 0x10 | 16 | ovl202 ovl203 ovl204 fdat160 fdat180 | Box16 x5, Func80062BD8Pose x2, V71DBox x2, Func801C8CBCBox, ... | lo, hi |
| struct 0x10 | 15 | ovl202 ovl203 ovl204 | LVtx x6, MVtx x6, V63Vert x2, V62Vert | sxy, otz, col, dp |
| struct 0x28 | 12 | ovl202 ovl203 ovl204 | PFT4 x10, PolyFT4w x2 | tag, rgbc, xy0, uv0, xy1, uv1, xy2, uv2, ... |
| struct 0x4 | 12 | ovl201 ovl202 ovl203 ovl204 fdat016 | Pt16 x3, Ang x2, DVEC x2, ViewWork, ... | pitch, yaw |
| struct 0x68 | 11 | ovl202 ovl203 ovl204 | LightSet x11 | lm, color, r, g, b, fog |
| struct 0x20 | 11 | ovl202 ovl203 ovl204 | PFT3 x10, PolyFT3w | tag, rgbc, xy0, uv0, xy1, uv1, xy2, uv2 |
| struct 0x80 | 10 | ovl201 | EmbView x9, Func8006C554Info | x0, y0 |
| struct 0x34 | 10 | ovl202 ovl203 ovl204 | PGT4 x10 | tag, rgbc0, xy0, uv0, rgb1, xy1, uv1, rgb2, ... |
| struct 0x28 | 10 | ovl202 ovl203 ovl204 | PGT3 x10 | tag, rgbc0, xy0, uv0, rgb1, xy1, uv1, rgb2, ... |
| struct 0x18 | 10 | ovl202 ovl203 ovl204 | DrawEnt x9, DrawEnt4 | t, mi, part, ws, lt |
| struct 0x8 | 10 | exe ovl201 ovl202 ovl204 | SVECTOR x4, Pos16 x3, StrmEnt, UVEC, ... | vx, vy, vz |
| struct 0x8 | 10 | ovl202 ovl203 ovl204 fdat002 | PList x4, HitRec x2, V67Ent x2, Func801C6944Hit, ... | fn, arg |
| struct 0x82 | 9 | ovl201 | EmbCursor x8, Func8006DCECSub | x, y |
| struct 0x1C | 8 | ovl201 ovl202 | PolyG3w x4, LineF4Loop x4 | tag, c0, xy0, c1, xy1, c2, xy2 |
| struct 0x1C | 8 | ovl202 ovl203 ovl204 | MPart x5, MdlObj x2, WModel | verts, n_verts, normals, prims, n_prims |
| struct 0x8 | 8 | ovl201 | Corner x8 | x, y |
| struct 0x8 | 8 | ovl201 | TexRect x8 | u, v, w, h |
| struct 0xFC | 7 | ovl202 | AcMoveCtx x7 | state, target, mode, tgt, goal |
| struct 0x18 | 7 | ovl202 ovl203 ovl204 | PartRec x7 | verts, nv0, nv1, nrm, flags, pkt, npkt |
| struct 0x10 | 7 | ovl201 ovl202 ovl203 | LineF2 x4, LitVtx, CVtx, PT2 | tag, c, xy0, xy1 |
| struct 0x8 | 7 | ovl201 fdat002 fdat010 fdat016 | CurWork x2, Func801C675CHit, Func801C5D04Hit, Func801C730CHit, ... | update, event |
| struct 0xA0 | 6 | ovl201 | EmbEdit x6 | hdr, img, cursor, view |
| struct 0x7C | 6 | ovl202 ovl203 | Part7C x5, Joint | model, world |
| struct 0x70 | 6 | ovl202 ovl203 ovl204 | LightSet x6 | lm, col |
| struct 0x64 | 6 | ovl201 | UiView2D x6 | x, y, ox, oy |
| struct 0x28 | 6 | ovl202 ovl204 | HudTag x6 | on, value, sx, sy, pos, icon |
| struct 0x28 | 6 | ovl202 ovl203 ovl204 | RecSet x3, V67RecSet x2, V63RecSet | n, total, rec |
| struct 0x20 | 6 | ovl201 | SprtPrim x5, Func80060168Prim | mode, sprt |
| struct 0x1C | 6 | ovl201 | EmbImage x6 | zoom, color, pixels, tool, step, x0, y0 |
| struct 0x18 | 6 | ovl202 | Corner x6 | uv, w, rgb, xy, z |
| struct 0x10 | 6 | ovl201 ovl202 ovl204 | LineF2 x6 | tag, r0, g0, b0, code, xy0, xy1 |
| struct 0x10 | 6 | ovl202 ovl203 ovl204 | CVtx x4, V67Vert x2 | sxy, otz, clip, near, col, lit, dp |
| struct 0xC | 6 | ovl202 | Vtx x6 | xy, z, rgb |
| struct 0x8 | 6 | ovl202 | FaceRef x6 | f, ot, arg, flags |
| struct 0x2 | 6 | ovl201 fdat018 | PadCfgWork x3, Range, Func8007B850Item, MailCond | first, count |
| struct 0xB4 | 5 | ovl201 | AsmObj x5 | head, w |
| struct 0x44 | 5 | ovl201 | CardFile x5 | blocks, name |
| struct 0x38 | 5 | ovl201 | AsmWork x5 | src, owner, b84, b85, b86, b87, sub, h90, ... |
| struct 0x34 | 5 | ovl203 ovl204 | V67GT x2, V62GT, V63GT, V63GT4 | tag, r0, g0, b0, code, xy0, uv0, c1, ... |
| struct 0x28 | 5 | ovl202 ovl203 ovl204 | V67FT x2, PolyFT4, V62FT, V63FT | tag, r0, g0, b0, code, xy0, uv0, xy1, ... |
| struct 0x22 | 5 | fdat022 fdat036 fdat038 fdat072 | Part x5 | f1c, f1e, f20, f21 |
| struct 0x18 | 5 | exe ovl201 ovl203 | CdlFILE x2, CdFile, Func80016678File, PolyF4w | pos, size, name |
| struct 0x18 | 5 | ovl202 ovl203 | PartRec x2, MPart x2, MdlPart | verts, nv, nrm, pkt, npkt |
| struct 0x16 | 5 | ovl201 | BoxOpen x5 | x, y, w, h, a, b, buf, tx, ... |
| struct 0x14 | 5 | ovl201 | SPRT x5 | tag, r0, g0, b0, code, x0, y0, u0, ... |
| struct 0x10 | 5 | ovl201 | TILE x5 | tag, r0, g0, b0, code, x0, y0, w, ... |
| struct 0x10 | 5 | ovl202 ovl203 | ScrVtx2 x2, DVert x2, LVtx | sxy, otz, dp |
| struct 0xC | 5 | ovl201 ovl204 | SprtRes x3, StrmHead, SprTex | t, tpage |
| struct 0x6 | 5 | ovl201 fdat040 | EmbCurWork x2, Func801C5CE0Def, PenWork, EmbCur | lo, hi |
| struct 0x160 | 4 | fdat004 fdat012 fdat016 fdat018 |  | pos, yaw, faction, dir, ai_mode, state, fn_state, mover, ... |
| struct 0x3C | 4 | fdat000 fdat016 fdat040 | Func801C5428World, Func801C7624World, Func801C76E4World, Func801C6020Obj | on_hit, stats |
| struct 0x26 | 4 | ovl201 | MailView x3, NameList | count, rows, top |
| struct 0x1C | 4 | ovl201 | LineF4Loop x4 | tag, r0, g0, b0, code, xy0, xy1, xy2, ... |
| struct 0x1C | 4 | ovl202 ovl204 | BNode x2, RibNode x2 | a, b, v0, v1, sxy, sz |
| struct 0x1C | 4 | ovl202 ovl203 ovl204 | MPart x3, MdlObj | verts, n_verts, n_extra, normals, extra, prims, n_prims |
| struct 0x18 | 4 | ovl201 ovl202 | PolyF3 x2, Seg32 x2 | tag, c, xy0, xy1, xy2, xy3 |
| struct 0x14 | 4 | ovl201 | SPRT x4 | tag, r0, g0, b0, code, x0, y0, t |
| struct 0xC | 4 | ovl201 | P_TAG x4 | addr, len, r0, g0, b0, code |
| struct 0x8 | 4 | ovl201 | SprtTex x4 | u0, v0, clut, w, h |
| struct 0x6 | 4 | ovl202 ovl203 fdat002 | Func801C6504Fx, BlastDmg, V768Shot, BlastDef, ... | kind, c, b |
| struct 0x6 | 4 | ovl202 | Shard6 x3, Tint6 | a, b, c, d |
| struct 0x4 | 4 | ovl202 ovl203 | ScaleItem x3, Ovl203Hit | kind, val |
| struct 0x160 | 3 | ovl202 ovl203 ovl204 | CObj3 x3 | px, py, pz, h0E, x10, h12, h36, dir, ... |
| struct 0xD4 | 3 | ovl202 ovl203 fdat022 |  | pos, yaw, faction, dir, state, fn_state, mover, vel, ... |
| struct 0xD0 | 3 | fdat002 |  | yaw, b41, fn, mover, count, spin, spin_acc |
| struct 0xD0 | 3 | ovl202 ovl203 |  | pos, y, arg, cb, timer, speed, accel |
| struct 0x60 | 3 | ovl201 | LetterLine x2, GarageEnt | c0, c1, c2, c3 |
| struct 0x60 | 3 | ovl201 | LetterLine x3 | text, pos, next, len |
| struct 0x3C | 3 | fdat008 fdat010 fdat182 | Func801C662CObj, Func801C5C6CObj, Obj | timer, def |
| struct 0x3C | 3 | ovl202 ovl203 ovl204 | HudWpn, Weap, WSlot4 | max, ammo, name |
| struct 0x3C | 3 | ovl202 ovl203 ovl204 | HudMode, Mode, WpnRec | kind, flags |
| struct 0x38 | 3 | ovl202 ovl203 ovl204 | Sect x3 | x, y, z, rot, min, max, flags, vis |
| struct 0x30 | 3 | ovl202 ovl203 ovl204 | Func80062BD8Entry x2, V62020Entry | pos, view, lo, hi, radius |
| struct 0x2A | 3 | fdat004 fdat160 fdat180 | Func801C6A64Node, Func801C8E08Node, Func801B8580Node | pos, lo, hi, id |
| struct 0x28 | 3 | ovl201 |  | name, attr, size, next, head, system |
| struct 0x24 | 3 | ovl202 ovl203 fdat000 | Func801C7DA8Model, Model, FMdl3 | id, parts |
| struct 0x20 | 3 | ovl202 ovl203 fdat014 | DPart3 x2, DPart4 | id, a, b, c, kind, d |
| struct 0x20 | 3 | ovl201 | MailMsgRow x3 | reply, id |
| struct 0x20 | 3 | ovl201 | PadRep x3 | latch, delay, rate, press, held |
| struct 0x20 | 3 | ovl202 ovl203 ovl204 | Func8005156CObj, Func80052238Obj, Func80051878Obj | flags, ang_b, ang_a |
| struct 0x1C | 3 | ovl202 ovl203 ovl204 | Func8005156CTarget, Func80052238Target, Func80051878Target | active, yaw, pitch |
| struct 0x1C | 3 | ovl202 | QuadRec x3 | uv0, uv1, uv2, uv3, nrm, v0, v1, v2, ... |
| struct 0x1C | 3 | ovl202 | Face x3 | w0, w1, w2, w3, n, v |
| struct 0x1C | 3 | ovl203 ovl204 | V67Rec x2, V63Rec | p00, p08, prims, nprim, idx, verts |
| struct 0x18 | 3 | ovl202 ovl203 ovl204 | CHit3 x3 | t, v10 |
| struct 0x14 | 3 | ovl202 ovl203 fdat014 | DPiece3 x2, DPiece4 | h12_pad, h12 |
| struct 0x14 | 3 | ovl201 | PolyF3 x3 | tag, c, xy0, xy1, xy2 |
| struct 0x14 | 3 | ovl202 | TriRec x3 | uv0, uv1, uv2, nrm, v0, v1, v2 |
| struct 0x14 | 3 | ovl202 | Face x3 | w0, w1, w2, n, v |
| struct 0x12 | 3 | ovl202 ovl203 ovl204 | CObjHead3 x3 | px, py, pz, h0E_dummy |
| struct 0x10 | 3 | exe | MemBlk x3 | used, size |
| struct 0x10 | 3 | ovl202 fdat004 fdat022 | Probe, Func801C60B0Hit, FloorHit | ws, h0e |
| struct 0x10 | 3 | ovl202 | ScrVtx x3 | sxy, otz, dp |
| struct 0x10 | 3 | ovl202 ovl203 | LVtx x2, LVert | sxy, otz, col, clip, dp |
| struct 0x10 | 3 | ovl202 ovl203 ovl204 | SprUV, BbTex, PanelTex | uv0, uv1, uv2, tri, uv3 |
| struct 0xC | 3 | ovl201 | PaintWork x2, PaintEditWork | part, slot, chan |
| struct 0xA | 3 | ovl202 | SndDef x2, HudPart | vab, prog, tone, note, prio |
| struct 0x8 | 3 | ovl202 fdat010 fdat024 | Tag, Func801C6498Blast, Func80077B78Blast | a, b, c |
| struct 0x8 | 3 | ovl201 ovl202 | BoxTag, P_TAG, ModelBody | addr, len |
| struct 0x8 | 3 | ovl201 | KnobWork x3 | owner, chan |
| struct 0x10D0 | 2 | ovl201 | MailGlobals x2 | order, row, work |
| struct 0x10D0 | 2 | ovl201 | MailGlobals x2 | sel, top, order, row, work |
| struct 0x66C | 2 | ovl201 | LetterWork x2 | tim_slot, label, label2, line, line2, seen, text, text2 |
| struct 0x264 | 2 | ovl201 | LetterWork x2 | label, line |
| struct 0x164 | 2 | ovl202 ovl203 | Func8007DB98Obj, HitObj3 | faction, dir, kind, ai_mode, mode, vel, fire_flags, flags, ... |
| struct 0x162 | 2 | ovl203 ovl204 | VEnt4 x2 | state, ap |
| struct 0x160 | 2 | fdat022 fdat036 | Ent x2 | part, f36, f3e, f3f, f40, f12x, target, f148, ... |
| struct 0x160 | 2 | fdat038 fdat072 | Ent x2 | part, f3e, f3f, f40, target, f148, f14a, f14b, ... |
| struct 0x160 | 2 | ovl202 ovl203 | MvObj3 x2 | pos, h2a, top, floor, faction, move, kind, state, ... |
| struct 0x110 | 2 | ovl202 ovl203 | TObj3 x2 | pos, mdl, tilt |
| struct 0x10C | 2 | fdat008 fdat016 | Func801C5418Obj, Obj | state, handler |
| struct 0xD4 | 2 | fdat034 fdat080 | Ent x2 | ang, f36, mode, state, fn, vx, ce, vz, ... |
| struct 0xD4 | 2 | ovl202 ovl203 |  | pos, yaw, h28, h2a, state, fn_state, mover, mode, ... |
| struct 0xD4 | 2 | ovl202 ovl203 |  | f08, f0A, dir, f2A, f2E, flags, state, arg, ... |
| struct 0xD4 | 2 | ovl202 ovl203 |  | set, h18, h1A, h1C, flags, dir, kind, cool, ... |
| struct 0xD2 | 2 | fdat022 fdat036 | Ent x2 | state, f42, cc, ce, d0 |
| struct 0xD2 | 2 | ovl202 ovl203 | Func80079AF8Obj, Func800785ACObj | vx, vz |
| struct 0x9C | 2 | ovl201 | EmbEdit x2 | color, ots |
| struct 0x72 | 2 | ovl202 | AcFall, AcJump | h28, h2a, h70 |
| struct 0x6E | 2 | ovl202 | AcSz x2 | r, h |
| struct 0x68 | 2 | fdat004 fdat018 | Func801C5E30Obj, Func801C50F8Obj | x2C, mode, x50, init, step, hit, x64 |
| struct 0x68 | 2 | ovl202 | LightSet x2 | light, color, unk62pad, r, g, b, fog |
| struct 0x64 | 2 | ovl201 | Layer x2 | draw, buf, size |
| struct 0x60 | 2 | ovl201 | LetterLine x2 | text, pos, len |
| struct 0x60 | 2 | ovl201 | LetterLine x2 | text, pos, next, len, ended |
| struct 0x4C | 2 | ovl201 | EmbWork x2 | zoom, zoom_prev, color, pixels, cursor, zoom_sel, answer |
| struct 0x3E | 2 | ovl203 ovl204 | Body, Ent | state, sub |
| struct 0x3C | 2 | fdat002 | Func801C63F8World, Func801C6504World | pos, min, max, on_hit, stats |
| struct 0x3C | 2 | fdat040 | Func801C5CE0Obj, Obj | y, mover, def |
| struct 0x38 | 2 | ovl202 ovl203 | V7BObj x2 | yaw, faction |
| struct 0x38 | 2 | ovl203 ovl204 | CurWs x2 | flags, lod |
| struct 0x34 | 2 | ovl201 | PolyGT4w x2 | tag, c0, xy0, uv0, clut, c1, xy1, uv1, ... |
| struct 0x34 | 2 | ovl204 | Fx x2 | state, blk |
| struct 0x30 | 2 | ovl202 ovl204 | FxEnt x2 | id, pos, uv, f2a, f2c |
| struct 0x30 | 2 | ovl202 | Sphere48 x2 | c, a, b, d, w, r |
| struct 0x30 | 2 | ovl202 ovl203 | TSlot, V7ASlot | target, lock, h8, hA, hC, hE, proj, aim, ... |
| struct 0x30 | 2 | ovl202 ovl203 | V7BSlot x2 | target, lock, t08, t0a, t0c, t0e, pos, aim, ... |
| struct 0x2C | 2 | fdat008 fdat016 | Func801C662CSlot, Func801C6904Link | kind, ent |
| struct 0x2C | 2 | ovl201 | SaveWork x2 | owner, status, cursor, mode, slot, tag, top |
| struct 0x2C | 2 | ovl201 | SaveWork x2 | owner, status, cursor, mode, slot |
| struct 0x2C | 2 | ovl202 ovl203 | AcSpawn, Ovl203Spawn | state, slot, ap, x, y, z, geom, rot, ... |
| struct 0x28 | 2 | fdat006 | Func801C5F00Obj, Func801C5F64Obj | state, timer, flags, count, first |
| struct 0x28 | 2 | ovl201 | PolyFT4w x2 | tag, c, xy0, uv0, clut, xy1, uv1, tpage, ... |
| struct 0x28 | 2 | ovl201 ovl202 | BoxFT4, PolyFT4 | tag, r0, g0, b0, code, x0, y0, u0, ... |
| struct 0x28 | 2 | ovl202 ovl203 | Ammo, FDef3 | kind, count, x24 |
| struct 0x26 | 2 | ovl202 ovl203 | CostObj, Ovl203Weapon | cost, flags |
| struct 0x24 | 2 | ovl201 | PolyG4w x2 | tag, c0, xy0, c1, xy1, c2, xy2, c3, ... |
| struct 0x24 | 2 | ovl202 | PartObj x2 | hdr, parts |
| struct 0x24 | 2 | ovl202 ovl203 | V7BRec x2 | mz_a, mz_b, lock_time, maxlock2, range, ywin2, pwin2, pwin, ... |
| struct 0x24 | 2 | ovl202 ovl203 | WpnSlot, WDef3 | kind, a2, a3, a4, a5, b0a, b0c, reload |
| struct 0x22 | 2 | fdat014 fdat016 | Def x2 | snd, arg |
| struct 0x22 | 2 | ovl201 | MailLine x2 | x, y, str |
| struct 0x20 | 2 | ovl201 | PolyFT3w x2 | tag, c, xy0, uv0, clut, xy1, uv1, tpage, ... |
| struct 0x20 | 2 | ovl201 | NameWork x2 | dst, arg, answer, mode, len, k |
| struct 0x20 | 2 | ovl202 | HitObj x2 | cb, flags |
| struct 0x1C | 2 | ovl201 | MsgWork x2 | owner, next, text, id, label, x, y, w, ... |
| struct 0x1C | 2 | ovl201 | TmdObj x2 | vert_top, n_vert, normal_top, n_normal, primitive_top, n_primitive, scale |
| struct 0x1C | 2 | ovl202 ovl204 | LNode, LineNode | a, b, sxy, sz |
| struct 0x1C | 2 | ovl202 | TgtScore x2 | def, dist, flags, dx, dz |
| struct 0x1C | 2 | ovl202 ovl203 | PadAxis, Ovl203Turn | x, y, dx, vel |
| struct 0x18 | 2 | fdat008 fdat160 | Func801C6AB0Obj x2 | pos, timer |
| struct 0x18 | 2 | ovl202 | PolyF4 x2 | tag, r0, g0, b0, code, x0, y0, x1, ... |
| struct 0x18 | 2 | ovl203 | V62Rec, V63Rec | p00, p08, prims, nprim |
| struct 0x14 | 2 | ovl203 ovl204 | Func80052238View, Func80051878View | pos, yaw |
| struct 0x13 | 2 | fdat002 fdat162 | Func801C5EF8Hit, Cnt | state, count, limit |
| struct 0x10 | 2 | fdat008 fdat160 | Func801C6AB0Timer x2 | tick, range |
| struct 0x10 | 2 | ovl201 | EmbImage x2 | color, pixels |
| struct 0x10 | 2 | ovl201 | PartPose, PartEntry | x, y, z, scale |
| struct 0x10 | 2 | ovl202 | ModelBody x2 | n, flags, verts |
| struct 0x10 | 2 | ovl202 ovl204 | Flare, BbObj | x, y, z, id, ws, kind, limit, period, ... |
| struct 0x10 | 2 | ovl202 ovl204 | VECTOR x2 | vx, vy, vz |
| struct 0x10 | 2 | ovl203 | PVert x2 | sxy, sz |
| struct 0xE | 2 | ovl203 fdat016 | Hit, Body | vx, vz |
| struct 0xC | 2 | ovl201 | MenuWork x2 | owner, arg, cursor |
| struct 0xC | 2 | ovl202 | RecHdr, ModelHdr | count, off |
| struct 0xC | 2 | ovl202 ovl203 | SprCol, SprTex | clut, tpage, u, v, w, h, col |
| struct 0xC | 2 | ovl202 ovl203 | V71DHit x2 | e, out |
| struct 0xB | 2 | ovl201 | PaintWork x2 | part, row |
| struct 0xA | 2 | fdat008 fdat182 | Func801C662CDef, Def | count, slot, a, b, c, d, e |
| struct 0x8 | 2 | ovl201 | EmbRgbW x2 | edit, chan |
| struct 0x8 | 2 | ovl202 | Blk8 x2 | b, col |
| struct 0x8 | 2 | ovl202 ovl203 | WSpread, FEnt3 | b0, b1, spread_x, spread_y |
| union 0x4 | 2 | ovl202 | SXY x2 | xy, s |
