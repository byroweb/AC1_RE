# Code archaeology: translation units, style clusters, mission lineage

Target **SLUS-01323** (code-identical to SCUS-94182). This is a bounded study of how the
retail code was organised and written: where the object files (translation units, TUs) of
the exe and the program overlays begin and end, how many coding styles the asm and the
already-matched C support, how the 47 story-mission units were cloned from each other, and
what the three in-game program builds (ovl202/203/204) changed between copies. Its last
section turns the findings into **matching priors** for the decomp queue.

Labels follow [README.md](README.md): **CONFIRMED** = read from the bytes or counted
exactly; **HYPOTHESIS** = an inference the bytes allow but do not prove. Authors are named
as style clusters (A/B/C). No cluster is tied to a person: the staff roll gives no per-system
roles (section 5).

All numbers come from the scripts in [`tools/archaeology/`](../tools/archaeology/) run on
2026-10-20 over `tools/loop/functions.tsv`. The scripts read the gitignored images through
`tools/loop/units.py`. Nothing here or in the committed TSVs contains game bytes: only
addresses, counts and offsets.

| script | output | what it does |
|---|---|---|
| `arch.py` | stdout | per-function asm features (shared reader for the others) |
| `tus.py` | `tu_map.tsv` | TU boundaries by contiguous modularity over the reference graph |
| `styles.py` | `tu_styles.tsv` | per-TU style rates, PAM clustering, 201-vs-202 permutation test; `--regions` prints the region table in section 2 |
| `csrc.py` | stdout | idiom counts in the matched C (`re/match/manifest.tsv`) |
| `missions.py` | `mission_lineage.tsv` | phylogeny and clone edges of the story-mission units |
| `divergence.py` | `twin_divergence.tsv` | aligns 202/203/204 in link order and classifies the twin bodies that differ |
| `doc_labels.tsv` | input | ~200 function addresses that existing docs tie to a subsystem (used to name TUs) |

```bash
python tools/archaeology/tus.py          # under a minute (O(n^2) DP; ovl201 has 1091 functions)
python tools/archaeology/styles.py       # + --regions
python tools/archaeology/csrc.py
python tools/archaeology/missions.py
python tools/archaeology/divergence.py
```

---

## Summary

1. **No link padding** (CONFIRMED). None of the 2871 overlay functions is followed by nop
   padding, and the exe's 20 padded ends all sit in the PsyQ block. psylink packed `.text`
   at 4-byte alignment, so object-file boundaries cannot be seen in the bytes. The TU map
   (section 1) is inferred from reference locality and is a **HYPOTHESIS** throughout. It
   has 12 exe TUs, 67 in ovl201 and 44/35/36 in ovl202/203/204.
2. **No dev filenames, and Wyrelade's splat layout has no TU splits** (CONFIRMED). His
   `configs/USA/*.yaml` give one `c` subsegment per unit, so there is no public TU list to
   check against.
3. **Three habit profiles, not one** (section 2). The clearest subject-neutral marker is
   the loop counter type. In the bulk of ovl201, **30–46 %** of `±1` counter steps are
   re-narrowed to `short`/`char` (cluster **A**). In ovl201's band 0x800866FC–0x8009A7E8
   the share is **7.7 %**, and that band also uses **signed** `char` loads (46 % of byte
   loads, against 8 % elsewhere; cluster **B**). In ovl202/203/204, the exe and all mission
   code it is **0–6 %**, with function-pointer methods and unsigned bytes (cluster **C**).
   The counts are CONFIRMED; reading them as different programmers is a HYPOTHESIS.
   Whole-TU clustering on all features is weak (silhouette ≤ 0.17), because subject matter
   (GTE, primitives, strings) dominates per-TU rates.
4. **The staff roll credits three programmers** (CONFIRMED, from STAFF.STR frames). It has
   *Main Programmer* Eiichi Hasegawa on his own card and *Programmer* Hiroyuki Arai and
   Masayuki Saitoh. Wikipedia lists only the last two. There is no per-system line, so
   A/B/C stay unnamed.
5. **Mission units come in two template families** (CONFIRMED by masked bodies):
   - A *library* family of 29 units shares a frozen object library, with one identical
     4912-byte dispatcher in 27 of them.
   - A *standalone* family of 18 units shares almost nothing.

   The standalone family holds the early story missions: median mission 14 against 28
   (permutation p = 0.0075). **M00 "Dummy00" has zero mission-specific bodies**: it is the
   bare library template. The pilot's 10 no-LOS units are all in the standalone family.
   Section 3 has the clone pairs.
6. **The 202/203/204 divergences are mostly build configuration, not late edits**
   (section 4). Of ovl202's 619 functions:
   - 423 are identical in all three builds.
   - 81 differ only between the mission build and the two versus builds.
   - 25 differ only in ovl204, which is split-screen: clip bounds 320→160 and 240→120, a
     shift of 4→3, and bigger primitive buffers.
   - 22 differ in all three.
   - 60 exist only in ovl202, and 8 more are missing from one of the versus builds.

   The real logic forks are the damage path (`0x80075350`), the "current player" pointer
   `0x80039D18` that 203/204 swap around calls, and the pre-START init block at
   `0x80082D04..0x800835A0`.

---

## 1. Translation-unit map

### Method (`tus.py`)

- **Nodes.** The unit's game functions in address order: exe `0x80011F28..0x80016EA4`
  (outside the PsyQ block), and every function in the overlays.
- **Edges.**
  - Each in-unit call or code pointer links caller and callee (weight 1).
  - Each `.rodata`/`.data`/`.bss` address used by 2–12 functions links those functions
    (total weight about |S|/2).
  - Addresses used by more functions are shared globals and are skipped.
- **Cut.** The exact dynamic program finds the contiguous partition that maximises Newman
  modularity. γ = 1 is the default; γ = 2 is given for the big TUs.
- **Why this works.** Static helpers, string literals, jump tables and file-scope statics
  pull a file together. Calls into other files' globals are discounted by the null model.

Checks:

| unit | functions | TUs (γ=1) | singletons | edge weight kept inside a TU | rodata interleaves |
|---|---|---|---|---|---|
| exe (game part) | 131 | 12 | 2 | 0.52 | 1 (`t00`/`t09`) |
| ovl201 | 1091 | 67 | 30 | 0.42 | 1 (`t39`/`t40`) |
| ovl202 | 619 | 44 | 16 | 0.69 | 0 |
| ovl203 | 579 | 35 | 16 | 0.72 | 0 |
| ovl204 | 582 | 36 | 17 | 0.75 | 0 |

**Independent hit (CONFIRMED match, HYPOTHESIS boundary).** The first file of the 202
family is ovl202 `t00`, which starts at 0x8004C1BC and has twins at ovl203 0x8004BE88 and
ovl204 0x8004BE30. It is the only multi-function body that ovl201 shares with the 202
family: its first two functions recur at ovl201 0x800A4EAC / 0x800A4F1C. The modularity
cut, which knows nothing about twins, starts ovl201 `t54` at exactly **0x800A4EAC**. So one
small file was linked first into the 202 family and in the middle of ovl201.

**Singletons.** These are functions with no in-unit references: they are reached only from
other units or through data tables. The DP cannot place them, and they are best read as
belonging to a neighbour.

**Multi-file TUs.** A few γ=1 TUs are clearly several files that call each other densely.
The γ=2 cuts inside them are listed under each unit as candidate internal boundaries.

### exe (game code 0x80011F28..0x80016EA4)

| TU | range | fns | bytes | doc systems |
|---|---|---|---|---|
| t00 | 0x80011F28–0x800125D0 | 6 | 1704 | `main`, game loop / overlay select, NOW-LOADING |
| t01–t02 | 0x800125D0–0x800131D0 | 6 | 3072 | collision primitives (sphere/box, swept segment-AABB) |
| t03 | 0x800131D0–0x80013510 | 9 | 832 | small s16 helpers |
| t04 | 0x80013510–0x80014930 | 28 | 5152 | GTE / light matrices |
| t05 | 0x80014930–0x80015440 | 19 | 2832 | math (ratan2 azimuth), reticle |
| t06–t08 | 0x80015440–0x80015580 | 5 | 320 | utils (`wmemset`) |
| t09 | 0x80015580–0x80016A3C | 51 | 5308 | CD / `.T` loader (MXT), checksum |
| t10 | 0x80016A3C–0x80016ACC | 2 | 144 | pad read |
| t11 | 0x80016ACC–0x80016EA4 | 5 | 984 | — (holds the `$gp` users 0x80016E14 / 0x80016E64) |

### ovl201 (front end)

Only TUs of at least 1500 bytes or with doc labels are listed; the full table is in
`tools/archaeology/tu_map.tsv`. "Style" refers to the clusters of section 2.

| TU | range | fns | bytes | doc systems | style |
|---|---|---|---|---|---|
| t00 | 0x8004DCC0–0x800500EC | 44 | 9260 | menu object pool (`obj_find_by_handle` region) | A |
| t08 | 0x80050804–0x80051868 | 28 | 4196 | — | A |
| t09 | 0x80051868–0x8005CA60 | 129 | 45560 | model renderer 0x80057190, GTE text renderer 0x80059B50, `box_init` | A |
| t18–t30 | 0x8005D754–0x8005F5E8 | 30 | 7828 | draw-cmd constructors (type-7 label ctor 0x8005D80C) | A |
| t31 | 0x8005F5E8–0x80066974 | 25 | 29580 | draw-cmd renderers types 1–8, `draw_string` / `draw_char` | A |
| t32 | 0x80066974–0x800689E0 | 16 | 8300 | — | A |
| t33 | 0x800689E0–0x80070834 | 79 | 32340 | — | A |
| t34 | 0x80070834–0x8007C68C | 63 | 48728 | mission launch store at 0x800714B8 | A |
| t35–t36 | 0x8007C68C–0x80083A58 | 78 | 29644 | name entry (pilot / AC name), `cursor_update` | A |
| t39–t40 | 0x80083C58–0x80086654 | 35 | 10748 | name entry; 0x80085640 uses `double` | A |
| **t43** | 0x800866FC–0x8008C088 | 84 | 22924 | free wrapper 0x8008878C, mission-number store 0x8008A890 | **B** |
| **t44** | 0x8008C088–0x800960C0 | 88 | 41016 | — | **B** |
| t45–t50 | 0x800960C0–0x80096684 | 8 | 1476 | four identical struct-copy leaves (`lwl/lwr` only) + small helpers | — |
| **t51** | 0x80096684–0x8009A7E8 | 70 | 16740 | — | **B** |
| t52 | 0x8009A7E8–0x800A46AC | 39 | 40644 | `main_frame_loop`, `render_dispatcher`, **AC stat builder 0x8009C964 (25,504 B)** | A |
| t54 | 0x800A4EAC–0x800A88F8 | 70 | 14924 | shared head file (above); mission/arena stores | A |
| t56 | 0x800A9068–0x800AE30C | 57 | 21156 | mission / mode-bit stores 0x800AABE0.. | A |
| t58 | 0x800AE7A4–0x800B1ED0 | 40 | 14124 | — | A |
| t61–t63 | 0x800B1F74–0x800B55E4 | 43 | 13936 | — | A |
| t65 | 0x800B597C | 1 | 120 | `GsMapModelingData` (libgs) | lib |
| t66 | 0x800B59F4–0x800B6DEC | 18 | 5112 | libgcc soft-float doubles (`__adddf3` .. `_err_math`) | lib |

γ=2 splits:

- t09 → 0x80051868 / 0x80057190 / 0x800572C4 / 0x80059C84 / 0x8005ACF4 / 0x8005B648
- t34 → 0x80070834 / 0x80072838 / 0x80073718 / 0x800740AC
- t52 → 0x8009A7E8 / **0x8009C964** (the stat builder plus one function, a file of its own) / 0x800A2D0C
- t54 → 0x800A4EAC / 0x800A5318 / 0x800A6EA0

Two corrections to `ac1_markup.tsv` found on the way:

- **`__builtin_delete` at 0x80088794 is a signature false positive** (CONFIRMED). It is the
  third instruction of a 44-byte "free if non-null" wrapper that starts at **0x8008878C**.
  ovl201 has no C++ evidence: there are no vtable thunks and only 0.07 indirect calls per
  1000 instructions.
- **Only two game functions use `double`** (CONFIRMED). They are the AC stat builder
  0x8009C964 (273 calls into the libgcc soft-float block) and 0x80085640 (8 calls). No exe or
  202-family code calls soft-float.

### ovl202 (mission build; ovl203/204 follow the same order)

| TU | range | fns | bytes | doc systems | γ=2 splits |
|---|---|---|---|---|---|
| t00 | 0x8004C1BC–0x8004C318 | 4 | 348 | shared head file (also in 203/204 and ovl201) | |
| t01 | 0x8004C318–0x8004DE88 | 28 | 7024 | `mission_set_result`, ovl202 main, chunk-5, XA radio player | |
| t12–t13 | 0x8004EC4C–0x8004FA44 | 13 | 3576 | movement integrator chain, PA pack loader, scene loader 0x8004F508 | |
| t14 | 0x8004FA44–0x80052338 | 21 | 10484 | HUD gauge/timer sites, geometry copy/relocate 0x80050FC4 | |
| t15 | 0x80052338–0x8005360C | 14 | 4820 | ready gate, sound-sequence parser/interpreter, text prompt | |
| t16 | 0x8005360C–0x800579D4 | 44 | 17352 | FDAT block loader, chunk 0-2/3/10 relocation, model-pack consumers, PA walker | |
| t17 | 0x800579D4–0x80067BAC | 96 | 66008 | PA emitters, object/level/AC renderers, missile steer | 0x8006260C, 0x80062FFC, 0x80064C3C |
| t18 | 0x80067BAC–0x8006D39C | 32 | 22512 | — (GTE-heavy; follows the hot transform 0x800674B0, which closes t17) | |
| t24 | 0x8006D9DC–0x800730DC | 66 | 22272 | world collision (bit grid, narrowphase, swept AABB, segment ray), section index | |
| t25–t32 | 0x800730DC–0x80073C2C | 19 | 2896 | chunk 8/9/11/12 handlers, prop collision, `acquire` 0x80073714 | |
| t33 | 0x80073C2C–0x80082D04 | 149 | 61656 | aim cone, radar, entity AABB, damage, enemy update, LOS, spawn/despawn, player think sets A/B | 0x80074620, 0x80077B78, 0x8007863C, 0x80079728 |
| t34 | 0x80082D04–0x800835A0 | 3 | 2204 | pre-START init block (diverges in all three builds) | |
| t36 | 0x800835E8–0x80089FE0 | 72 | 27128 | damage router, projectile thinks, FX spawners, pool allocator | |
| t37–t40 | 0x80089FE0–0x8008B320 | 21 | 4928 | primary objective VM, timer, end sequence, TICK driver, QUERY | |
| t41 | 0x8008B320–0x8008C6FC | 5 | 5084 | secondary actor-thread VM | |
| t42 | 0x8008C6FC–0x8008D8DC | 11 | 4576 | HUD markers, "#Location Now" | |

HYPOTHESIS: t33 is about five files. Its γ=2 cuts fall between the doc-labelled groups:

- aim cone and radar scan (0x80073C2C–)
- entity AABB collision, damage and enemy update (0x80074620–)
- AI fire, LOS and node FX (0x80077B78–)
- spawn framework, active set and mission tick (0x8007863C–)
- player thinks and handler sets A/B (0x80079728–0x80082D04)

---

## 2. Style fingerprints and clusters

### Features (`arch.py`, `styles.py`)

Per function, counted from the words:

- sign-extension pairs (`sll/sra 16`, `sll/sra 24`) and masks (`andi 0xffff/0xff`)
- signed vs unsigned byte and half loads, and the byte/half/word share of data accesses
- **counter steps** (`addiu r,r,±1`) followed within 3 instructions by a re-narrowing:
  this is a `short` or `char` loop variable
- jump tables vs compare-against-constant branches
- `jalr` and indirect calls through a word loaded from an object (method pointers)
- stack traffic outside the prologue, saved registers, unfilled delay slots
- function length, GTE ops, mult/div, `$gp`, frame pointer

They are split into STYLE habits and SUBJECT features (GTE, mult/div, memory and call
density, loops, `lwl/lwr` copies) so that subject is not mistaken for author.

### Uniform compiler facts (CONFIRMED)

- **No function sets up `$fp`** in any unit: `-fomit-frame-pointer` (or -O ≥ 1) everywhere.
- **No `break` divide checks** anywhere: the `div` macros were not expanded with trap
  checks.
- **`$gp` is used only by 6 exe functions:** 0x800122A8 (exe `t00`), 0x800163F8 /
  0x800164A0 / 0x80016678 (exe `t09`, the `.T` loader) and 0x80016E14 / 0x80016E64 (exe
  `t11`). All are gp-relative accesses to `.sdata` with gp = 0x80039C5C. No overlay function
  touches `$gp`. HYPOTHESIS: at least these exe files were compiled with `-G8` (or similar);
  other exe files may be too and simply never touch a small global.

### Whole-TU clustering is weak

Robust z-scores were computed on 16 STYLE rates for the 33 TUs of at least 1000
instructions (exe 2, ovl201 20, ovl202 11), then clustered with PAM:

| k | silhouette |
|---|---|
| 2 | 0.117 |
| 3 | 0.156 |
| 4 | 0.171 |
| 5 | 0.163 |

No k gives well-separated clusters. The 201-vs-202 split is real but small: between/within
distance ratio 1.06, permutation **p = 0.022** (2000 relabellings). Per-TU rates are
dominated by subject. For example, ovl201 `t31` (draw-cmd renderers) has 314 stack
accesses and 153 `lwl/lwr` per 1000 instructions, because it builds primitives on the
stack. So the count of styles is read from fixed regions instead (`styles.py --regions`,
whole-region sums):

| region | fns | insns | narrow counters | `sext16` /1000 | `andi 0xff` /1000 | signed byte loads | byte share | method calls /1000 | saved regs /fn |
|---|---|---|---|---|---|---|---|---|---|
| exe game code | 131 | 5087 | 0.00 | 3.3 | 2.6 | 0.08 | 0.12 | 0.79 | 2.1 |
| ovl201 0x8004DCC0–0x800866FC | 552 | 57999 | **0.38** | 9.4 | 6.6 | 0.09 | 0.28 | 0.05 | 2.9 |
| ovl201 0x800866FC–0x8009A7E8 | 250 | 20539 | **0.08** | 4.6 | 5.6 | **0.47** | 0.40 | 0.00 | 1.7 |
| ovl201 0x8009A7E8–0x800B597C | 270 | 27749 | **0.46** | 8.4 | 4.5 | 0.08 | 0.35 | 0.14 | 2.7 |
| ovl202 0x8004C1BC–0x80073C2C (loaders, render, collision) | 356 | 40604 | 0.03 | 3.2 | 0.4 | 0.04 | 0.14 | 0.15 | 3.1 |
| ovl202 0x80073C2C–0x8008A048 (entities, player, combat) | 226 | 22791 | 0.16 | 2.9 | 1.4 | 0.03 | 0.18 | **2.11** | 3.7 |
| ovl202 0x8008A048–0x8008D91C (VMs, HUD) | 37 | 3637 | 0.00 | 2.8 | 5.2 | 0.00 | 0.08 | 3.02 | 2.9 |
| mission library bodies (≥10 units) | 90 | 8379 | 0.07 | 3.3 | 1.1 | 0.04 | 0.34 | 1.79 | 2.6 |
| mission-specific bodies (1 unit) | 166 | 10757 | 0.02 | 3.5 | 0.9 | 0.00 | 0.18 | 2.05 | 2.0 |

Per TU, the counter habit holds inside each region:

- ovl201 A-region TUs: 0.19–0.72
- ovl201 `t43` / `t44` / `t51`: 0.07 / 0.12 / 0.04
- ovl202 TUs: 0.00–0.05, except `t33` and `t36` at 0.16–0.17

### Clusters (counts CONFIRMED, authorship HYPOTHESIS)

| cluster | where | habits |
|---|---|---|
| **A** | ovl201 except the B band: menus, draw commands, text, name entry, launch screens, garage stat builder | `short`/`char` loop counters (≈40 %); many `sll/sra 16` (s16 arithmetic locals); unsigned bytes with `andi 0xff` re-narrowing; almost no function pointers (0.05 method calls per 1000 instructions); heavy stack use for on-stack primitives; the only `double` user |
| **B** | ovl201 0x800866FC–0x8009A7E8 (`t43`, `t44`, `t51`; no doc labels yet except the mission-number store 0x8008A890) | `int` counters (8 %); **plain (signed) `char`** for byte data (`lb` = 47 % of byte loads); short functions (82 instructions on average) with ~1.7 saved registers; no GTE, no method pointers |
| **C** | ovl202/203/204, the exe game code, all mission overlays | `int` counters (0–6 %); unsigned `u8`/`u16` fields; **function-pointer methods** (`obj->think(obj)`: 2–3 indirect calls per 1000 instructions in the entity/VM code); GTE inline macros in render/transform; in ovl202, more saved registers per function (3.3 against 2.5 in ovl201) |

C has a visible sub-profile, C′: ovl202 0x80073C2C–0x8008A048 (entities/player/combat) has
16 % narrow counters and twice the stack traffic of the loader/render half. That could be a
second hand inside the mission build, or simply code that juggles more s16 vectors. It is
left as part of C.

The evidence therefore supports **two robust styles (A vs C) and a probable third (B)**.
That fits the three programmers on the roll. It does not prove three people, and it does
not say who wrote what.

The pilot's claim that "0x8005B12C–0x8005E710 looks like 202-family code" is **not
supported** by these habits. That band (`box_init`, the draw-cmd constructors) has 60 %
narrow counters and 24 `sext16` per 1000 instructions, which is strongly A-like.

### Source idioms in the matched C (`csrc.py`)

Distinct hand-matched files only: twins share one file, and the 46 m2c drafts committed by
`tools/loop/sweep.py` are skipped because they carry m2c's idioms. The sample is 42 ovl201
files (31 A, 11 B), 24 ovl202-family, 6 exe and 64 mission-library files. Rates are per 100
non-blank body lines.

| group | `s16`/`short` | `s32`/`int` | `u8` | `goto` | `switch` | `else` | `->` | asm/volatile barriers | `#define` |
|---|---|---|---|---|---|---|---|---|---|
| cluster A | 3.01 | 2.12 | 2.48 | 0.18 | 0.53 | 1.24 | 24.8 | 1.42 | 3.01 |
| cluster B (11 tiny files, mostly getters) | 0 | 3.57 | 2.68 | 0 | 1.79 | 0 | 13.4 | 0 | 0 |
| cluster C, all | 2.93 | 4.99 | 6.03 | 2.03 | 0.81 | 3.19 | 48.3 | 0.15 | 0.09 |
| C: ovl202-family | 3.70 | 9.32 | 4.98 | 1.13 | 0.16 | 3.54 | 31.7 | 0.48 | 0.48 |
| C: exe | 0.55 | 13.8 | 0.55 | 6.63 | 0 | 0 | 22.1 | 1.10 (`register … asm("$28")`) | 0 |
| C: mission library | 2.91 | 3.36 | 6.65 | 1.93 | 1.02 | 3.33 | 54.0 | 0 | 0 |

These counts agree with the asm on the type habit:

- The A files declare `short` more often than `int` and use `#define`s and asm/volatile
  barriers.
- The C files are `int`-dominant and use `->` two to four times as often (object and
  method code).

The sample is small and biased toward short functions. `goto` in the exe and mission files
mostly comes from forced matches. Treat this table as weak support only.

---

## 3. Mission-unit lineage (`missions.py`)

Each of the 47 story units (FDAT_T entries 0..98 even, without 32/58/78) is reduced to:

- its set of relocation-masked bodies (the `twins.py` identity)
- its template counts: calls to `has_los` 0x800781E8, `los_ray` 0x8007816C, aim cone
  0x80073C2C and `acquire` 0x80073714; text size; and whether it has the 4912-byte
  dispatcher

Distance is 1 − byte-weighted Jaccard over bodies. The tree is UPGMA. A **clone edge** for
unit X is the unit holding the largest share of X's mission-specific bytes (bodies present
in at most 3 units). Full table: `tools/archaeology/mission_lineage.tsv`, with the Newick
tree in its trailer.

### Two families (CONFIRMED)

| family | units | text | shared code | template |
|---|---|---|---|---|
| **library** | 29: M00 M04 M08 M10 M13 M17 M20–M28 M30 M31 M33 M34 M38 M40 M41 M43–M49 | 21–35 KB, 45–85 functions | one frozen object library. The 4912-byte dispatcher (12 jump tables) is **byte-identical after masking in all 27** units that have it; M04 and M27 carry a 3468-byte variant instead (also 12 tables; 617 of its 867 words appear in order in the 4912-byte one) | LOS 1×, or 3× with cone 4× |
| **standalone** | 18: M01 M02 M03 M05 M06 M07 M09 M11 M12 M14 M15 M18 M19 M32 M35 M36 M37 M42 | 3–14 KB, 13–63 functions | little: small groups and clone pairs only | LOS 0–2× |

- The pilot's 10 units that never call LOS (M02 M03 M05 M06 M11 M12 M14 M18 M36 M37) are
  all standalone.
- The pilot's "LOS 3× / cone 4×" family (M04 M21 M25 M28 M30 M31 M41 M43) is one tight
  library sub-tree (distances ≤ 0.2 apart from M04).
- **Distances inside the standalone family** are mostly 0.4–0.9; only the clone pairs
  are closer (for example M01/M32 0.30).
- **Story order:** standalone missions have median mission number 14, library missions 28
  (difference of means 12.0, permutation p = 0.0075). Consecutive mission numbers are no
  more alike than random pairs (mean distance 0.674 vs 0.700).

### Which template looks earliest (HYPOTHESIS)

- **M00 "Dummy00" is the library template itself** (CONFIRMED). It contains no body that is
  absent from every other unit (0 mission-specific bytes). M10 and M13, the two "Prototype
  MT Test" missions, are M00 plus 64 and 396 bytes (distance 0.02 between them).
- **The standalone template is older than the library** (HYPOTHESIS). The standalone units
  dominate the early story, they never adopted the library, and their LOS/cone use is the
  simplest. If missions were built roughly in story order, the shared library was factored
  out after about mission 19 and every later mission was cut from M00.
- **M04/M27 are the oldest library branch** (weak HYPOTHESIS). Their dispatcher is a fork of
  the frozen 4912-byte one and is otherwise library code. Which version came first cannot be
  read from the bytes: each also has words the other lacks.

### Clone pairs and borrowings (share of the left unit's mission-specific bytes found in the right unit)

| unit | from | share | note |
|---|---|---|---|
| M27 "AC Battle" | M04 "Terrorist Pursuit" | 98 % of 3816 B | old-dispatcher pair |
| M04 | M26 "Destroy Plane Computer" | 94 % of 4012 B | |
| M15 "Exterminate Organisms" | M07 "Exterminate Organisms" | 95 % of 6428 B | same name, same code |
| M30 / M31 "Eliminate Squatters" | each other | distance 0.06 | same name; M30 adds cone/acquire calls |
| M35 "Stop Security MTs" | M19 "Destroy Space Catapult" | 100 % of 816 B | |
| M03 "Rescue Survey Team" | M12 "Stop Gas Exposure" | 85 % of 1964 B | |
| M45 "Stop Security MT" | M18 "Remove Base Occupants" | 85 % of 2752 B | **cross-family**: a standalone mission's object code reused in a library mission |
| M25 "Destroy Intruders" | M12 | 100 % of 980 B | cross-family |
| M30 | M34 "Eliminate Strikers" | 100 % of 1360 B | |
| M24 "Destroy Plus Escapee" | M20 "Destroy Base Generator" | 83 % of 1196 B | |

The late missions M45, M46, M48, M49 (plus M20) form their own library sub-tree, each with
1.5–3.2 KB of mission-specific code.

---

## 4. Divergent twins across ovl202 / ovl203 / ovl204 (`divergence.py`)

The three builds are aligned in link order on masked bodies. Functions in replaced blocks
are paired when their coarse skeleton similarity is at least 0.5. Each pair is classified
as `size`, `frame`, `const` (same opcodes, other immediates) or `code` (instructions
inserted, deleted or replaced).

### Patterns over ovl202's 619 functions (CONFIRMED)

| pattern | functions |
|---|---|
| identical in all three | 423 |
| 202 differs, 203 == 204 (mission vs versus) | 81 |
| only ovl204 differs (202 == 203) | 25 |
| all three differ | 22 |
| only in ovl202 | 60 (+7 missing in 204 only, +1 in 203 only) |

Code that exists only in ovl202 is about 26–31 KB. It is the sound-sequence and text-prompt
file (`t15`), the HUD markers (`t42`), the QUERY/end-result evaluation (`t40`), the secondary actor VM
(`t41`) and 13
functions of the entity/player file (`t33`). The versus builds add 21 or 30 functions of
their own: mode main loops and loaders, plus in ovl204 the AC instantiate path 0x8008156C
(doc: "called twice", one per player).

### What the differences are

1. **Mode macros, not edits** (CONFIRMED readings of the diffs).
   - ovl204 (two-player split-screen) halves the screen clip bounds in every
     render/transform function that tests them (`slti 321→161/159`, `241→121`,
     `16→8`, `sll 4→3`).
   - ovl204 enlarges the primitive buffers (0x1C000/0x1E000 → 0x21000/0x23000) and adds
     one more buffer pointer, in 0x800572A8.
   - Both versus builds replace entity-count loop bounds (`li 16 → 2`, `li 32 → 2`).
   - The result fade is 100 frames in ovl202 (`mission_set_result` 0x8004C318) and 90 in
     both versus builds.
   - These are one source tree compiled with different `#define`s.
2. **Player generalisation.** In **46** functions both versus builds load a
   *current-player* pointer from `0x80039D18` (`lui 0x8004; lw -25320`) where ovl202 does
   not. Examples: the PA/scene loaders 0x8004F1A8, HUD 0x800501DC, the damage path
   0x80075280/0x80075350, the 0x80079370–0x8007A0E4 block, and every player think and
   installer from 0x8007A654 to 0x800826FC. This is `#if VERSUS` code or a later refactor
   that made the player slot a variable.
3. **Real logic forks** (HYPOTHESIS: the latest-edited code in the build).
   - **Entity damage apply** 0x80075350: 200 / 268 / 276 bytes.
     - In ovl202 it takes four arguments and indexes an `s16` table.
     - In both versus builds it drops one argument, indexes the table as words, and saves,
       sets and restores `0x80039D18` around the hit call.
     - ovl203 (link cable) picks the opponent by comparing against `*(0x80039D30)+916`;
       ovl204 (split-screen) compares against the fixed entity addresses 0x80040FA0 and
       0x80040FA0+368.
   - **Pre-START init block**, ovl202 `t34` 0x80082D04 / 0x80082E7C / 0x80083038:
     376/372/432, 444/444/484 and 1384/1440/1488 bytes. This is the largest three-way
     difference: about 128 replaced words in 0x80083038.
   - **Render dispatchers** 0x80062BD8 (1060 / 1148 / 872 bytes) and 0x8006131C (3044 /
     2044 / 2044 bytes): about 1000 bytes of mission-only render path removed from both
     versus builds.
   - **Damage router** 0x800835E8 (336 / 352 / 380 bytes), **player targeting**
     0x8007DB98 (880 / 968 / 964 bytes), **missile/rotate helpers** 0x8005715C..0x80057360
     (only ovl204 differs).

Reading for "late edits" (HYPOTHESIS): items 1 and 2 are configuration and say nothing
about timing. Item 3 is where the source really diverged. The damage-apply and pre-START
init forks are the best candidates for changes made after the versus modes were split off,
because both versus builds carry *different* new code there rather than one shared
`#if VERSUS` branch.

---

## 5. Staff roll (STAFF.STR)

Read from frames that jPSXdec decoded into a scratch directory; no frames were committed
(CONFIRMED).

| card | names |
|---|---|
| Executive Producer | Naotoshi Zin |
| Producer | Yasuyoshi Karasawa |
| Mechanic Designer | Syoji Kawamori |
| **Main Programmer** | **Eiichi Hasegawa** |
| Director | Toshifumi Nabeshima |
| **Programmer** | **Hiroyuki Arai, Masayuki Saitoh** |
| Graphics Designer | Masahiro Kajita, Kaichi Satoh, Nozomu Iwai, Sakumi Watanabe |
| 3D Modeler / Animator / CG Movie / Sound / Music | further cards, then the US staff (localisation, QA, voice cast) |

- No card gives a per-system programming role, so A/B/C are **not** mapped to names.
- What the roll does establish is three credited programmers, one of them senior ("Main").
  That is consistent with the three habit profiles of section 2. Pairing them, for example
  the main programmer with the shared engine (C), would need a developer interview or a
  role breakdown; it is not attempted here.

---

## 6. Matching priors per cluster

These are concrete levers for `drive.py` packages and the match-function skill. Each comes
from the measured habit, not from a successful match yet, so treat it as the first thing to
try, not a rule.

### Cluster A (most of ovl201)

- **Declare loop counters and small locals as `s16` (or `u8`), not `int`.** About 40 % of
  counter steps are re-narrowed (`addiu; sll 16; sra 16`). An `int` draft will miss those
  pairs, and a cast on the use site does not reproduce them in GCC 2.7.2. Change the local's
  type.
- Byte fields are **`u8`** (`lbu`) with `& 0xff` on arithmetic results. Write `u8 x = …;`
  temporaries rather than masking by hand.
- Dispatch is by `switch` or `if` chain, almost never by function pointer: a `jalr` in an
  A function is rare enough to suspect a library call. Jump-table use per instruction is
  the same as in C (0.57 vs 0.55 per 1000 instructions), so the pilot's "more jump tables
  in 201" does not hold once the counts are normalised.
- High stack traffic plus `lwl/lwr` means **primitives and RECTs built as on-stack structs**
  and struct assignment (`*dst = *src`) for 8/12-byte copies. Use a local `struct` and assign
  it whole.
- The two `double` users (0x8009C964, 0x80085640) need literal `double` arithmetic. Keep the
  conversions (`(double)int`, compare, `(int)`) in source order; each becomes a libgcc call.

### Cluster B (ovl201 0x800866FC–0x8009A7E8)

- **Plain `char`** (signed: `lb`) for byte data and locals. Swapping `u8` for `char` (or
  `s8`) is the first lever on a sign-extension diff here.
- **`int` loop counters.** Short functions with one or two saved registers: avoid extra
  temporaries that would spill to `s2`/`s3`.

### Cluster C (ovl202/203/204, exe game code, mission overlays)

- **`int` counters**, `u8`/`u16` struct fields, and `s16` only where the struct field is s16
  (sign-extension pairs come from the loads, not from locals).
- **Method calls through the object** (`o->think(o)`, the `+0x4C`/`+0x58`/`+0x5C` slots of
  `ENTITY_AI_FRAMEWORK.md`): write them as calls through a struct field. A `jalr` after an
  `lw` from `a0`/`s0` is the signature.
- **Twin builds:** a function matched in ovl202 should be retried on its 203/204 twin with
  only the mode constants changed:
  - entity loops 16 → 2 (or 32 → 2)
  - screen 320/240 → 160/120 and `<<4` → `<<3` (ovl204 only)
  - fade 100 → 90 frames
  - player slot 0 → `*D_80039D18` (both versus builds)

  This covers the 81 "mission vs versus" and 25 "ovl204-only" patterns. The 22 "all three
  differ" functions need their own C.
- **Mission units:** one C file for the library serves up to 27 units (frozen library). The
  M04/M27 dispatcher and the standalone family need their own files. Within a clone pair
  (M07/M15, M30/M31, M10/M13, M04/M27) replay the matched C with `twins.py` and expect only
  constant or offset edits.
- **`$gp`** (exe only: 0x800122A8, 0x800163F8, 0x800164A0, 0x80016678, 0x80016E14,
  0x80016E64) uses `.sdata` with gp = 0x80039C5C. Under the pinned `-G0`, use the
  `register u32 *gp_base asm("$28")` device of `re/match/exe/func_800164A0.c`, or try a
  `-G8` build of the TU with the small globals declared at their real size.

### All clusters

- No frame pointer and no divide trap checks are needed anywhere. If a draft produces
  either, the flags are wrong, not the C.
- A TU boundary from `tu_map.tsv` is a hint for where `static` helpers live. A callee
  inside the same TU that has no other caller is a good candidate for a `static` function
  defined above its caller (GCC 2.7.2 emits in source order).

---

## Open questions / follow-ups

- **Cluster B has no doc labels** apart from the mission-number store. Naming the systems
  in ovl201 0x8008C088–0x8009A7E8 (`t44`, `t51`) would show whether B is a subsystem
  (shop? garage parts lists?) or a person.
- **Validating the TU cuts:** the strongest test is matching two neighbouring functions
  across a predicted boundary with shared `static` data. One TU's statics must be laid out
  in definition order.
- **Direction of the 3468 → 4912 dispatcher fork:** a word-level alignment of their 12 jump
  tables could show which cases were added.
