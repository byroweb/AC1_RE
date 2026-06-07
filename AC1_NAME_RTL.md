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

Changes 2–4 modify runtime-built structures; baking them needs their init-builder
sites (next pass). Change 1 is a static overlay-code patch and is baked.

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
