# Plant view — a live "factory floor" of the AC1 mission loop (`tools/plantview/`)

A dashboard that draws the mission lifecycle, the entity table and the per-frame
pipeline of **SLUS-01323 v1.1** as stations with pipes between them, colours the
station the game is in, animates a token along a pipe when a transition is observed,
and shows the counters and gauges behind it (mission stage, mode byte, entity
counts, player AP, end-sequence counter). Every address it reads is one the docs
already list, with the docs' own CONFIRMED / HYPOTHESIS label carried through to
the page.

```
stations.json   layout: fields (addr/type/status/doc), arrays, derived values, stations, transitions
poller.py       samples RAM (live DuckStation MCP or offline save states), serves JSON + SSE on 127.0.0.1
index.html      single self-contained canvas page (no build, no CDN); scrubber for replay
```

## Run

```bash
python tools/plantview/poller.py --source synthetic --open      # no emulator, no game data: a scripted walk through the lifecycle
```

```bash
python tools/plantview/poller.py --source states --open         # replay: one sample per .sav in $AC1_STATES_DIR, ordered by frame
```

```bash
python tools/plantview/poller.py --source live --open           # DuckStation (sadnescity MCP fork, port 2346) read_memory
```

Then open <http://127.0.0.1:8765/> (or pass `--open`). Options: `--rate` (samples/s
for replay pacing), `--port`, `--state <path|glob>` (repeatable; default = the whole
library), `--disc` (savestate RAM anchor, default `$AC1_BIN`; a cached anchor in
`$AC1_STATES_DIR/disc_cache.json` also works), `--record out.jsonl` (keep recordings
out of the repo or under `scratch/`), `--source jsonl --replay out.jsonl`, `--once`
(one decoded sample as JSON, no server — handy for scripting).

Needs Python 3.10+ and, for `states`, the `zstandard` package (see
`tools/duckstation/savestate.py`). The MCP client is the one in
`tools/duckstation/radio_line.py` (urllib only).

### Page controls
- **LIVE** follows the newest sample; dragging the scrubber, **◀ / ▶** or **←/→**
  pauses on a sample; **▶ play** / space steps through the history at 4 Hz; `l`
  returns to live. The history is kept by the poller (`--history`, default 5000
  samples), so a live run can be scrubbed back too.
- Hover a field row for the doc reference and note. `C` / `H` badges = the docs'
  CONFIRMED / HYPOTHESIS status of that address, station or pipe (dashed pipes are
  HYPOTHESIS).
- Tokens: yellow = lifecycle transition, blue = spawn (template consumed or a slot
  came alive), orange = damage seen on a live entity, red = an entity reached AP 0,
  grey = a slot was freed. The grey marching token on the per-frame row is purely
  illustrative (the §4 order) — those stations have **no live probe yet**.

## How the state is decoded (honesty notes)
`poller.decode_sample()` reads the `fields` and `arrays` of `stations.json` and derives:

| derived | rule | status |
| --- | --- | --- |
| `lifecycle` | `OUT_OF_MISSION` if mode byte ≠ 0 or objobj ptr = 0; `ENDING` while end counter ≠ 0; `EXIT` once a result bit (`0x380`) is set and the counter is back to 0; `LOAD` while objobj is bound but slot 0 (player) is not; `OBJECTIVES_COMPLETE` when `0x801D0B50 == 3`; else `RUNNING` | HYPOTHESIS (the RUNNING→ENDING→EXIT writers are CONFIRMED in MISSION_STATE_MACHINE.md §5; LOAD/EXIT boundaries are heuristics) |
| `entities_active` | slots whose `+0x00` points into the template table **and** whose think method `+0x58` is non-null | HYPOTHESIS |
| `entities_collide` | active slots with a geometry record ptr at `+0x04` | HYPOTHESIS |
| `entities_dead` | active slots with AP `+0x160 == 0` | HYPOTHESIS |
| `templates_total / consumed` | template records with HP ≠ 0 / with `+0x00 ≠ 0` | HYPOTHESIS (in M30 the player's own record is not marked, so "consumed" undercounts by one) |

Live reads are coalesced into contiguous `read_memory` regions of at most 16 KB
(the fork times out on larger reads). Once a breakpoint has been used in a
DuckStation session every MCP call takes ~2 s, so a sample can take ~20 s; a fresh
emulator session samples in well under a second. `--status-every N` reduces the
`get_status` calls (they supply the emulator's frame number).

## Adding a station
1. **Field**: add an entry under `"fields"` with `addr`, `type` (`u8/s8/u16/s16/u32/s32`),
   `status`, `doc`, `label`, optional `note`, `bits` (for flag words) or `gauge`.
   The poller reads it automatically; it shows up in the field table.
2. **Station**: add an object to `"stations"` with a unique `id`, `label`, `group`
   (`lifecycle`, `entity` or `frame`), grid `col`/`row`, `doc`, `status`, the `fields`
   it displays (first three are printed in the box), and either
   - `active_when: {"lifecycle": "<STATE>"}` — lit while the derived lifecycle equals it, or
   - `counter: {"value": "<derived>", "of": "<derived>|number", "label": ...}` plus
     `active_when: {"counter_gt": 0}` — a big `v / of` counter and bar.
   Optional `gauges: [{"field", "min", "max", "label"}]` draws bars from raw fields,
   `substeps` / `note` are documentation carried with the station.
3. **Transition**: add `{from, to, label, status, doc}` to `"transitions"`; `bend: 1`
   routes the pipe below the row (for skips and return paths), `-1` above. Tokens run
   along a transition when the lifecycle moves from `from` to `to`; the entity-row
   events (`spawn`, `tick`, `death`, `free`) are wired in `index.html`'s
   `diffAndAnimate()`.
4. **New derived value**: compute it in `decode_sample()` and describe it under
   `"derived"` in `stations.json` so the page can label its status.
5. Check with `python tools/plantview/poller.py --source synthetic --once` (decodes
   without an emulator) and reload the page — it fetches `stations.json` on load.

To give a per-frame station a live field, add the field and list it under that
station; the poller does not yet sample within a frame (that needs a breakpoint or
the fork's `trace` tool), which is the next step for that row.

## Scope / what it is not
A first prototype: no persistence, no authentication (it binds 127.0.0.1 only), no
in-frame sampling, and the entity classification above is a reading of the RAM map,
not a trace of the allocator. Nothing here contains or writes game data; recordings
(`--record`) hold decoded field values only, but keep them out of the repo anyway.
