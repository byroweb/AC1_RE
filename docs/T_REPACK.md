# AC1 `.T` repacker — writing containers back

Target **SLUS-01323 (v1.1)**. The write side of the `.T` container format: the
inverse of `tools/extract/extract_t.py` / `tools/pa/pa_obj.py`. This is the foundation
primitive for every edit path (mission text → maps → custom geometry), named as
the "next concrete step" in `docs/AC1MOD_VISION.md`. Companion docs:
`MXT_LOADER.md` (container + checksum RE), `REFERENCE.md` §3/§6, `PA_FORMAT.md`.

Tool: `tools/extract/t_repack.py`. Harness: `tools/extract/t_repack_test.py`.

## What it does (and its boundary)

Operates on the **flat, de-sectored image** — 2048 user-bytes per sector, exactly
what `extract_t.py` produces. It does **not** do raw 2352-byte disc-sector
reframing (sync/header/subheader/EDC/ECC) or reinjection into the `.bin`; that is
a separate downstream step (jPSXdec / `psxinject`). See *Next steps* below.

```
disc .bin ──extract_t (de-sector)──► flat .T ──unpack──► [entries]
                                          ▲                   │ edit
   reinject (jPSXdec/psxinject) ◄──raw-sector──◄── flat .T ◄──pack──┘
```

## Container format (recap)

Sector 0 = TOC, a packed `uint16` array of per-entry **start sectors**:

- **count-first** (every AC1 `.T`): `u16[0]` = entry count `N`, then `N+1`
  start-sectors. Entry `i` = `flat[off[i]*2048 : off[i+1]*2048]`.
- **offset-first**: `u16[0]` = first entry's start sector (`==1`); auto-detected.
- Entries are sector-aligned; **zero-length entries** (repeated offsets) are
  common and preserved.
- The TOC's trailing bytes after the offset table are zero padding (verified
  across all 90 containers); `t_repack` preserves them verbatim regardless.

### Terminator quirk (`MIS.T`)

The final offset word is usually the end sentinel `== total sectors` (so it
tracks file growth). **`MIS.T` stores `0`** as its final word — the last declared
entry decodes empty (`off[N-1] → 0` is a non-increasing pair). `t_repack`
captures the final word literally: a real sentinel is recomputed on edit; a
literal terminator (the `0`) is preserved, so round-trip stays byte-identical.

## The checksum (critical)

Every **nonempty** entry ends with a trailing `uint32` checksum word — game
routine `FUN_80015b24`, modelled in `re/ac1_mxt`:

```
checksum = 0x12345678 (seed) + sum of every 32-bit word EXCEPT the last
```

stored in the last word. **An edited entry with a stale checksum hangs the game
at "NOW LOADING."** `replace_entry()` recomputes it automatically (pass
`--no-fix-checksum` to opt out).

Ground truth on the pristine disc (`--check-checksums`):

| container | nonempty entries | valid checksum |
|-----------|-----------------:|---------------:|
| `GG/COM/FDAT.T` | 123 | 123 / 123 |
| `GG/P0/PA00.T`  | 114 | 114 / 114 |
| `GG/MS/MIS.T`   | 547 | 546 / 547 (one text entry's final word is payload, not a checksum) |

The Python `checksum()` is a port of `re/src/ac1_mxt.c`; the harness builds the C
model and asserts they agree byte-for-byte, so the two never drift.

## Usage

```sh
# Prove the round-trip on one container (rebuilds the TOC, compares vs source):
python3 tools/extract/t_repack.py --file GG/COM/FDAT.T --verify-roundtrip

# Audit checksums:
python3 tools/extract/t_repack.py --file GG/MS/MIS.T --check-checksums

# Replace an entry (checksum auto-fixed) and write a new flat .T:
python3 tools/extract/t_repack.py --file GG/COM/FDAT.T --replace-entry 4 new_entry.bin -o out.T
```

Python API (`import t_repack`): `unpack(flat) -> Container`,
`pack(Container) -> bytes`, `replace_entry(c, i, payload)`,
`checksum/verify/fix_checksum(entry)`.

## Validation gate

`python3 tools/extract/t_repack_test.py` must pass before anything downstream is built:

1. **Round-trip gate** — `pack(unpack(x)) == x`, byte-identical, for **all 90**
   `.T` containers on disc (TOC is *rebuilt* from entry sizes, not echoed).
2. **Checksum parity** — Python `checksum()` == C `ac1_mxt_checksum` on sampled
   real entries.
3. **Edit correctness** — `replace_entry` rewrites one entry's checksum, leaves
   every other entry's payload byte-identical, and the result re-verifies.

Current status: **all three PASS (90/90 byte-identical).** A worked
mission-text edit (`MIS.T` requester string) round-trips with a valid checksum
and only the edited entry changed.

## Next steps (not in this tool)

- **Disc reinjection:** raw 2352-byte sector reframing + EDC/ECC, and the
  **grow-past-span risk** — an edited file larger than its allocated sector span
  (`disc_files.json` `sector_first..sector_last`) needs the CD image rebuilt, not
  an in-place patch.
- **PA block encoder:** OBJ/glTF → sub-object table + vertex/prim records, the
  inverse of `pa_parser.py` / `pa_obj.py`, for authoring level geometry.
- **MIS mission-parameter editing** (spawns/rewards/time-limit) beyond text.
