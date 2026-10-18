# AC1 Pristine RE project (`AC_1_USA_2048_RE`)

A clean, **unmodified** Ghidra project for the USA PS1 v1.1 build (SLUS-01323),
intended to be **shared** without any Farsi-translation artifacts. The Farsi
working project is now `AC_1_USA_2048_FARSI`; this one mirrors all of its
reverse-engineering knowledge against pristine bytes.

## Layout

| Block | Range | Source | Notes |
|-------|-------|--------|-------|
| `CODE` | `0x80011000–0x80039fff` | `SLUS_013_2048.23` (PS-X EXE) | Resident exe, imported via the **PSX Executables Loader** (`ghidra_psx_ldr`), language `PSX:LE:32:default`, PSYQ signatures on. |
| `overlay` | `0x8004ada0–0x800cf59f` | `disc_map/overlays/ovl201_front_end.bin` | `front_end` overlay carved from FDAT.T entry 201 (the menu/text/name code — `draw_string` etc.). |

The PSX loader also creates the standard PS1 hardware/RAM blocks (I/O, DMA, SPU,
CACHE, RAM). `AddRawBlockAnalyze.java` carves the overlay out of the big
uninitialized RAM block at its real load base, so the whole `front_end` image is
mapped. The only `Unable to read bytes` warnings left are reads of unmapped BSS
(`0x8003a000–0x8004ad9f`) and high RAM (`0x801xxxxx`) — harmless.

> **Earlier builds** (before 2026-10-17) used a DuckStation RAM dump
> `overlay_80050000.bin` at `0x80050000–0x800dffff`; it left `0x8004ada0–0x8004ffff`
> unmapped and produced `Unable to read bytes at 0x8004xxxx` warnings inside
> overlay functions. The carved disc entry supersedes it (see below).

## Overlay provenance (why it's still "pristine")

`ovl201_front_end.bin` is **FDAT.T entry 201** carved straight from the pristine
disc by `python tools/extract/extract_overlay.py --disc` (542,720 bytes, trailing
checksum verified, load base `0x8004ada0` per `disc_map/overlays/overlays.json`).
Nothing but the game's own load logic is applied, so the bytes are pristine by
construction. It is gitignored (`disc_map/overlays/`) — carve it locally.

The original project used `overlay_80050000.bin`, a **DuckStation RAM dump**
(2026-06-06) of the same overlay once loaded. That dump was lost in the 2026-10
machine migration. It had been verified pristine against the Farsi patch
footprint (884 patched bytes, all inside entry 201; of the 830 that fell in the
dump window, 826 carried the pristine byte, 0 the patched byte, 4 were runtime
data drift). The carved entry covers `0x8004ada0–0x800cf59f`, i.e. all of the
dump's code plus the `0x8004ada0–0x8004ffff` head the dump lacked; the dump's
tail past `0x800cf5a0` lies beyond the on-disc entry (runtime memory, not file
content) and no markup entry pointed into it (`ApplyMarkup` reports no `no-func`).

## Markup transfer

Knowledge is Ghidra **markup** (names, prototypes, comments) keyed by address —
not C source. Because the resident bytes are identical, it transfers 1:1:

- `ghidra_scripts/DumpMarkup.java` — exports non-default function names,
  prototypes, calling conventions, all listing comments, and user labels from
  `AC_1_USA_2048_FARSI` to a TSV (full address strings, overlay-safe).
- `ghidra_scripts/ApplyMarkup.java` — replays the TSV into this project.
- `ac1_markup_pristine.tsv` (committed, 1204 lines) is the neutralized dump and
  is the input to use — the FARSI project is no longer available, so the
  DumpMarkup step below is historical.
- Result: **1061 functions, 1058 prototypes, 143 comments** (3 prototypes
  reference absent types — re-run after Parse C Source if you want them).
  Reproduced 1:1 on 2026-10-17 with Ghidra 12.1.4 / ghidra_psx_ldr 2026.10.03 and
  the carved overlay (`protoFail=3`, `labels=0`, no `no-func`/`bad-addr` lines).
- Two Farsi-flavored names were neutralized for sharing:
  `farsi_gte_renderer_type6 → gte_renderer_type6`,
  `draw_kanji_RECLAIMED_shape_name → draw_kanji`.

## Regenerate from scratch

Prerequisites: `GHIDRA_INSTALL_DIR` (zip install with `ghidra_psx_ldr` in
`Ghidra/Extensions`), `JAVA_HOME`, `AC1_BIN` (your SLUS-01323 v1.1 `.bin`), and
`disc_map/overlays/ovl201_front_end.bin` from
`python tools/extract/extract_overlay.py --disc`. Keep the project **outside the
repo** and outside any cloud-synced folder (Ghidra lock files + OneDrive do not
mix); on the Windows box that is `AC1_GHIDRA_PROJECT_DIR=C:\Users\bross\dev\ac1_ghidra`.

```bash
HL="$GHIDRA_INSTALL_DIR/support/analyzeHeadless"   # Windows/Git Bash: …/analyzeHeadless.bat
PROJ="${AC1_GHIDRA_PROJECT_DIR:-$HOME/ac1_ghidra}"  # project dir, outside the repo
cd <repo>
rm -rf "$PROJ"/AC_1_USA_2048_RE.gpr "$PROJ"/AC_1_USA_2048_RE.rep "$PROJ"/AC_1_USA_2048_RE.lock*

# 0. carve the resident exe out of the disc image: sectors 39–121 = 83 × 2048 B
#    of user data (169,984 B) from the MODE2/2352 image — disc_map/disc_files.json
#    entry "SLUS_013.23". Written as SLUS_013_2048.23 so .gitignore matches it.
python - <<'EOF'
import os, sys, struct
sys.path.insert(0, "tools/extract")
from extract_t import read_sectors
d = read_sectors(os.environ["AC1_BIN"], 39, 83)
assert d[:8] == b"PS-X EXE", d[:8]
pc0, gp0, t_addr, t_size = struct.unpack_from("<4I", d, 0x10)
assert (t_addr, t_size) == (0x80011000, 0x29000), hex(t_addr)   # CODE 0x80011000–0x80039fff
out = os.path.join(os.environ.get("AC1_GHIDRA_PROJECT_DIR", os.path.expanduser("~/ac1_ghidra")), "SLUS_013_2048.23")
open(out, "wb").write(d); print("wrote", out)
EOF

# 1. (historical) dump markup from the FARSI project — skip; use the committed TSV
#"$HL" "$PWD" AC_1_USA_2048_FARSI -process -readOnly -noanalysis \
#  -scriptPath "$PWD/ghidra_scripts" -postScript DumpMarkup.java "$PWD/ac1_markup.tsv"
#sed -e 's/farsi_gte_renderer_type6/gte_renderer_type6/g' \
#    -e 's/draw_kanji_RECLAIMED_shape_name/draw_kanji/g' ac1_markup.tsv > ac1_markup_pristine.tsv

# 2. import pristine exe + overlay + apply markup (~3 min)
"$HL" "$PROJ" AC_1_USA_2048_RE -import "$PROJ/SLUS_013_2048.23" \
  -scriptPath "$PWD/ghidra_scripts" \
  -postScript AddRawBlockAnalyze.java "$PWD/disc_map/overlays/ovl201_front_end.bin" 0x8004ada0 overlay \
  -postScript ApplyMarkup.java "$PWD/ac1_markup_pristine.tsv"
```

Expected log lines: `Using Loader: PSX Executables Loader`,
`Using Language/Compiler: PSX:LE:32:default:default`,
`ADDBLOCK 'overlay' @ 8004ada0..800cf59f len=0x84800 init=true`, and
`APPLYMARKUP functions=1061 prototypes=1058 protoFail=3 comments=143 labels=0`.

> **Naming gotcha:** `.gitignore` matches `SLUS_013_*.23` and `SLUS01323*` but
> **not** the literal disc name `SLUS_013.23`. Write the carved exe as
> `SLUS_013_2048.23` (as above) or keep it outside the repo, or git will offer to
> commit the game binary.

> **Windows/Git Bash:** call `analyzeHeadless.bat` directly from Bash with
> forward-slash paths; do not wrap it in `cmd //c "..."` (the quoting breaks).
> The `.bat` reads `GHIDRA_INSTALL_DIR`/`JAVA_HOME` from the user environment.
> Opening the project in the GUI: `ghidraRun.bat <proj>/AC_1_USA_2048_RE.gpr`
> (a desktop shortcut "Ghidra - AC1 RE" does exactly this on the Windows box).

### Smoke test with GhidraMCP (headless, no GUI)

`ghidra-mcp --headless` serves the saved program in-process via PyGhidra. It
opens the program named after `--binary`, so pass the same `SLUS_013_2048.23`
path and it reuses the analyzed, marked-up program instead of re-importing:

```bash
ghidra-mcp --headless --project "$PROJ" --project-name AC_1_USA_2048_RE \
  --binary "$PROJ/SLUS_013_2048.23" --discard
```

`tools/ghidra/ghidramcp_smoke.py` does this end to end (needs the `mcp`
package, which the ghidra-mcp venv has): `get_program_info` reports
`PSX Executables Loader` / `PSX:LE:32:default`, `search_functions_by_name
draw_string` returns `draw_string @ 8006599c` (inside the overlay block), and
`decompile_function draw_string` returns C. Gotcha baked into the script: a
client that launches the server with `mcp`'s `StdioServerParameters` must pass
`env=dict(os.environ)` — the default child environment drops
`GHIDRA_INSTALL_DIR` and PyGhidra refuses to start.

> **Headless gotcha (snap install):** the PSX loader/language lives in the snap's
> settings dir, so `analyzeHeadless` (run outside the snap sandbox) only finds it
> when `XDG_CONFIG_HOME` points at `~/snap/ghidra/current/.config`. Without it you
> get `Language not found for 'PSX:LE:32:default'`. Use the **12.0** headless
> (`snap current` → 35); the snap's 12.1 lacked the extension. A zip install with
> `ghidra_psx_ldr` in `$GHIDRA_INSTALL_DIR/Ghidra/Extensions` needs none of this.
