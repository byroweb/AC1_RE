# Mission-runtime overlay dump (FDAT 202)

`mission_overlay_FDAT202_80050000.bin` — raw RAM dump of the swappable FDAT
overlay region while a **mission was running**, captured 2026-06-14 from
DuckStation (game paused, in-mission via save slot 2).

- **Load / base address:** `0x80050000`
- **Size:** 589824 bytes (`0x90000`) → covers `0x80050000–0x800DFFFF`
- **Verify:** file offset `0x28CFC` = `E0 FF BD 27` (`addiu sp,sp,-0x20`) =
  spawn-init `0x80078CFC`.

## Why this exists
`0x80050000–0x800DFFFF` is a **swappable overlay region** — different code
chunks (garage/UI overlay 201, mission-runtime overlay 202, …) load into the
SAME addresses at different times. The Ghidra project's database has a DIFFERENT
overlay's bytes in this range, so the live mission spawn/script functions
(traced via DuckStation) don't match Ghidra (byte mismatch verified at
`0x80083894`). This dump is the *mission* overlay so it can be analyzed/annotated
correctly. The always-resident main exe (`0x80011000–0x80039FFF`) is unaffected
and already matches Ghidra.

## How to import into Ghidra (do NOT overwrite the existing program)
Two safe options:
1. **New program** (recommended): File → Import File → this .bin →
   Language `MIPS:LE:32:default` (R3000, little-endian) → set image base
   `0x80050000` → analyze. Keeps the existing program untouched.
2. **Overlay block** in the existing program: Memory Map → Add Block → name it
   e.g. `mission202`, set as an **Overlay** block at `0x80050000`, import bytes
   from this file. Annotations on the base block stay separate.

Backup of both Ghidra projects (pre-import) is at
`../_ghidra_backup_2026-06-14/` (RE + FARSI .rep databases, 23 MB).

## Functions confirmed live in this overlay (annotate after import)
From DuckStation RE (see memory `project_ac1_dynamic_spawn`,
`project_ac1_mission_runtime`):
- `0x8008B42C` — mission **script-VM dispatch** loop (opcode stream ~`0x8014FCD8`).
- `0x8008B830` — **"spawn group N" opcode**: `s1 = 0x8019FAB8 + N*44`; if
  flag!=1 → call `0x80078CFC`.
- `0x80078CFC` — **spawn routine** (a0 = spawn record): allocates slot, loads
  resources, inits record, sets spawned flag.
- `0x80078A2C` — enemy-slot **allocator** (returns AC-array record, base
  `0x801A26B8` stride `0x170`).
- `0x800788C8` — **resource loader** (copies+relocates geometry/collision via
  `FUN_80050FC4`).
- `0x80078B14` — **binder** (sets spawn-record flag=1, stores slot index).
- `0x80078C00`-ish — record **initializer** (type ptr `0x8019F590`, AI/collision
  defaults, back-links to spawn point).

Ghidra auto-analysis had broken/missing function boundaries here; the dump lets
those be created cleanly.
