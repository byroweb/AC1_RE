# AC1 Ravens' Nest DATA screen — round-trip proof + Farsi-localization handoff

Status (2026-06-08): the memory-card round-trip is **proven** and the pilot name
**سلام** renders correctly in-game after a full cold-boot + card reload (see the
shaped lam-alef ligature in `docs/screens/salaam_zoom.png`). The remaining work,
for a fresh chat, is to **Farsi-localize the DATA screen labels** (still English).

| screenshot | what it shows |
|---|---|
| `docs/screens/data_screen_salaam.png` | the full DATA screen: سلام (Farsi, correct) + English labels |
| `docs/screens/salaam_zoom.png` | zoom of سلام (shaped, ligature) — the round-trip visual |
| `docs/screens/load_data_list.png` | the LOAD DATA list (NO./AC NAME/SORTIES/CREDITS) |

## Quick resume (DuckStation save state)
- **Slot 2 = this exact DATA screen**, on the baked `[RTL]` disc, save loaded from
  the memory card, pilot = سلام. `load_state` slot 2 to land right here.
- Slot 10 = name screen with سلام typed (pre-confirm). Slot 1 = post-confirm test
  briefing. Slots 5/6/7/9 = various name-entry screens.
- Memory card slot 0 file `BASCUS-94182A` holds the سلام save (title
  `ARMOREDCORE01 SORTY000`).

## How to get here from a cold boot (if no save state)
1. `boot_game .../Armored Core (v1.1) [RTL].cue`. Let the PS / FromSoftware /
   ASSEMBLE-SYSTEM intro play; press Start to reach the "Push Start Button" title.
2. **Beat the demo reel.** The title flips to the attract/demo reel after a few
   idle seconds. Use `tools/emu/ac1_mcp_input.py` (or the `input_sequence` MCP tool):
   **2 Start presses with a ~120-frame (2 s) gap** steps demo→title→main menu.
   A 1 s gap is too fast (reel re-arms). The title is the only demo-reel race.
3. Main menu: **Scenario Mode** (Cross) → submenu **ادامه (Continue)** (Cross) →
   **SLOT 1** (Cross) → LOAD DATA list → save row (Cross) → "Load the saved data?
   **YES**" (Cross). Loads into the Ravens' Nest hub.
4. In the hub carousel, rotate to the **memory-card/DATA** category and Cross.
   (From the ARENA category it's **Right ×2**: ARENA → mail(laptop) → DATA card.)

## The DATA screen — what to localize (the actual task)
Left menu (English → Farsi needed):
`SAVE DATA`, `LOAD DATA`, `SAVE EMBLEM`, `LOAD EMBLEM`, `OPTIONS`.
Right info panel:
- pilot name **سلام** — already correct (Farsi, shaped). Leave it.
- `CREDITS`, `RANKING` (shows a number, e.g. 15), `MISSION REPORT`,
  and the report rows `Sorties`, `Success`, `Failure`, `Overall` — all English.

These are drawn by the same `draw_string`/`draw_confirm` engine documented in
[memory] project_ac1_string / project_ac1_font. The Farsi text for each label can
be produced with the `farsi-translate` skill (it already generated نام خلبان /
فاصله / تمام). Localizing = find each label's string/source (likely overlay-
resident, like the name screen's labels) and replace with shaped Farsi bytes,
respecting RTL placement and the overlay checksum (project_ac1_overlay_checksum).
The LOAD/SAVE list headers `NO. / AC NAME / SORTIES / CREDITS` and the confirm
prompts (`Do you want to create new data?`, `Load the saved data?`, `YES/NO`) are
also English and on the same screens.

## Memory-card round-trip (already proven) — see AC1_NAME_SHAPER.md
- Write side: card file holds `e0 f5 ad 3e` (سلام) **verbatim at offset 0x216**;
  serializer copies a staging record (live `0x80031BE6` read-watch never fires;
  staging copies @`0x8011D5EA/D62A/E46E/E4AE`). No transform.
- Read side (cold boot → load): RAM clean after boot → after load
  `0x80031BE6` and game-state `0x801F50AA` both = `e0 f5 ad 3e`. Same bytes
  `draw_string` renders as سلام (this screen). Round-trip byte-identical.
- Gotcha: wiping a name buffer with `ff ff ff ff` kills its `0x3e` terminator →
  `draw_string` runs off the end → **hang**. Keep/restore a terminator.

## Tooling
- `tools/emu/ac1_mcp_input.py` — DuckStation MCP handshake + frame-timed
  `input_sequence`; default 2 Start presses (use `--gap 120`). Note: the
  DuckStation MCP is single-session, so this script's session evicts the agent's
  (recover with any `get_status`). For agent-driven nav, call `input_sequence`
  directly instead to avoid the churn.
