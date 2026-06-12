# AC1 Pilot-Name Screen — Right-to-Left (Farsi) Layout

Reverse-engineering notes + patch for turning the Armored Core (SLUS-01323)
pilot-name entry screen into a right-to-left Farsi layout. Companion to
`farsi_name_rtl.c` (the patch as C) and `build_rtl_patch.py` (the disc bake).
Screen is **320×240**, glyph advance **16px**, name field = **8 columns**
(X = 8,24,…,120), max name length **8**.

## UI model
The screen runs two passes per frame over the same `0xD0`-byte element objects:
- **Update loop** (`0x8009BCE0`): calls each element's update method at `+0x74`.
- **Render dispatcher** (`~0x8009C07C`): switches on element **type** (`+0xa`);
  **type 7 → `draw_string`** (ASCII font, `\0`-terminated), **type 6 →
  `0x80059b50`** (Farsi-aware: bytes ≥ 0x80 → Farsi glyphs, `0x3e`-terminated).

Each element holds **X at `+0x48`, Y at `+0x4c`** (signed halfwords), positioned
**relative to its parent box**. Active elements: cursor box `0x801A7318` (type 3),
"PILOT NAME" label `0x801A73E8` (type 7), name field `0x801A74B8` (type 6,
string = name buffer `0x80031BE6`).

## The four changes
1. **Right-justify name + reverse cursor** — rewrite the cursor's update method
   `0x80083A28`. Original `cursor_X = n·16 + 8` (LTR) → write both
   `name_X = 136 − n·16` and `cursor_X = 120 − n·16` each frame, where `n` =
   glyph count (`*0x801A28EE`). 48-byte in-place patch. **Baked & verified
   booting from disc.**
2. **Move box right** — `box_init` (`0x8005B648`, args X/Y/W/H) builds the window
   border as cached prims at screen-init; children move with it. PILOT NAME box
   X 24 → **168** aligns its right edge with the keyboard panel (24+288=312).
3. **Localize label** — flip label element type 7 → 6 (`0x801A73F2`) to route
   through the Farsi renderer, string `0x8004C6F4` "PILOT NAME" → `نام خلبان`.
4. **Translate keys** — keyboard row "SPC END" (`0x800823D4`) → `فاصله` / `تمام`.

**All four changes are now baked and verified booting from a fresh disc.** The
runtime-built structures (changes 2–4) are handled by small **name-screen-specific
trampolines** in free overlay space, so the shared builders are never altered for
other screens:
- **Box (2):** the shared box caller `0x8005C290` is redirected to a hook that
  forces X=168 **only when Y==178** (the PILOT NAME box); keyboard panel and
  memory-card slot boxes pass through unchanged.
- **Label (3):** flipping the type during construction crashed the render
  (half-built element). Instead the **stored type is poked to 6 every frame**
  (and the label right-justified, X=72) from `cursext`, tail-called by the
  cursor's update method — a routine that only runs on the name screen, after
  setup. String `0x8004C6F4` is overwritten in place with `نام خلبان`.
- **Keys (4):** the SPC/END row string is relocated to free space `0x800820E8`
  and the row's strptr-table entry `0x800B8614` repointed (`فاصله` … `تمام`).

## Baking (FDAT entry 201)
Overlay = FDAT entry 201, runtime base **`0x8004ADA0`**, flat base
**25,548,800** in the extracted `.T`. `runtime → flat = 25,548,800 + (rt −
0x8004ADA0)`. Entry 201 ends with a checksum word (seed `0x12345678` + sum of
preceding words) the loader verifies — recompute it after any edit or the game
hangs at NOW LOADING. `build_rtl_patch.py` patches the `.T`, fixes the checksum,
and reinserts the FDAT sectors (Mode2/Form1, 2048B @ offset 24) into a disc copy.

## Shaping Farsi strings
Use the `farsi-translate` workflow: `farsi_runtime_shape.py` turns a Persian
string into draw-order glyph bytes (validated against the build-time shaper).
`نام خلبان` → `E4 81 84 DE 9F 20 E0 81 E5` + `3E`.

## Status & caveats (2026-06-07)
**Done:** the complete RTL Farsi pilot-name screen — RTL name + reversed cursor,
box moved right, `نام خلبان` label (right-justified), `فاصله`/`تمام` keys — is
baked and boots from a clean disc. `build_rtl_patch.py` is the executable source
of truth (every patch carries an `EXPECT` guard on the original bytes + recomputes
the entry-201 checksum). `farsi_name_rtl.c` documents each change as C.

**Caveats (read before building on this):**
- This is **patch-level RE**, not a full decompilation of the menu engine. Every
  function/field/offset *touched* is documented (cursor.update, box_init + its
  shared caller, the type-7 text constructor, the render dispatcher, the element
  struct, the keyboard row glyph format), but surrounding code is not exhaustively
  mapped.
- The box move is **functional rather than elegant**: `box_init`'s argument
  passing is messy, so the patch hooks the *caller* (keyed on Y==178) rather than
  decompiling `box_init` itself.
- The label fix is a pragmatic **per-frame stored-type poke** (via `cursext`), not
  a "proper" type-6 element constructed from scratch. It is stable and crash-free,
  but it relies on the cursor update method running every frame on this screen.
- A blanket per-glyph keyboard shift was tried and **reverted** (it threw the
  alphabet, esp. `ا`, out of alignment); only surgical single-glyph X-spacer
  nudges (`ا`,`آ`) are baked. Keyboard rows are otherwise at stock spacing.
- **Not done:** the per-keystroke runtime **name shaper** (so typed names join and
  reverse correctly). Until then the name field shows isolated glyphs in typed
  order. Tracked separately.
