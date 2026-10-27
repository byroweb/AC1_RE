# Play-coverage sweep: what to play, and what to type

Every AC1 function is byte-matched. The matched C shows *how* each function works, but
often not *what it is for* in the game. This sweep records which functions run in which
situation while you play. A later naming pass uses that record to name and comment the
functions, in three tiers:

- **confirmed**: play and code agree;
- **static**: clear from the code alone (maths, draw and sound helpers);
- **unclear**: needs more play.

Only confirmed and static names go into `include/ac1/symbols.tsv`.

The tool is `tools/duckstation/coverage_tag.py`; its options are in
`tools/match/README.md` under "Coverage tagging from play". This page is the playing
checklist.

## Before each session

1. Start DuckStation (the MCP fork, port 2346) and boot the game, or load a state.
2. Make sure no other Claude session is driving DuckStation. The MCP server takes one
   client at a time, so starting the script ends anyone else's session.
3. Health check: the server answers and the right units show as loaded.
   ```
   python tools/duckstation/coverage_tag.py --check
   ```
4. Start the tagger in a terminal, with the label of where you are now:
   ```
   python tools/duckstation/coverage_tag.py --context title
   ```
   With no other options it watches every code unit: the exe, ovl201..204 and all 58
   mission overlays. It arms all functions of whatever is loaded, and switches overlays by
   itself when the game loads a mission or returns to the garage. You never need a
   different command for a different part of the game.

## While you play

- **Every time the situation changes, type the label in the tagger's terminal and press
  Enter.** Use the labels below, word for word. The naming pass groups by label, so
  `shop buy` and `buying parts` would count as two different screens.
- Labels describe the **activity**, not the mission. The script already records which
  mission overlay was loaded, so `combat` in mission 30 and `combat` in mission 12 are
  told apart for free.
- Type `?` for a status line: armed and queued breakpoints, functions hit, loaded units.
  Type `q` to stop. It always removes its breakpoints and resumes the game.
- The game stutters for a moment after each new label, while code that hasn't been seen
  under that label hits its breakpoint once. That is the data arriving.
- **If the game crawls for more than a few seconds** (a whole overlay armed), stop with `q`
  and restart with `--max-armed 300`. The script then keeps 300 breakpoints armed and
  rotates the rest in every 10 s.
- Doing the same thing twice under the same label is free: functions already seen under
  that label (in this or any earlier run) are not armed again. Functions seen under 8 or
  more labels are not armed again at all; they run almost everywhere.

## Sessions and labels

Roughly 20–30 minutes each, in any order. A label in **bold** is the one to type. Do
what you can; a skipped row is just a gap the naming pass marks as unclear.

### 1. Front end and garage

| label | type it when |
|---|---|
| **boot** | from power-on: logos, opening movie |
| **title** | title screen |
| **new game** | starting a new game, up to the first garage screen |
| **pilot name** | entering the pilot name |
| **ac name** | entering the AC name |
| **load** | loading a game from the memory card |
| **save** | saving to the memory card |
| **options** | options / config menu |
| **controller config** | changing a button assignment |
| **garage** | garage top menu |
| **assembly** | assembly screen: browsing parts, swapping parts |
| **check performance** | the AC spec / performance screen |
| **shop buy** | parts shop, buying |
| **shop sell** | parts shop, selling |
| **paint** | colour / paint screen |
| **emblem** | emblem editor |
| **ac test** | AC test arena |
| **mail** | mail list and reading mail |
| **ranking** | ranking screen, if there is one |
| **mission list** | choosing a mission |
| **briefing** | the mission briefing, up to accept |

`live_probe.py` (struct layouts, see `tools/match/README.md`) fits into this session.
Run it after `q` on the tagger: it starts its own MCP session.

### 2. Missions: the core loop

Play two or three missions, typing labels as the mission goes:

| label | type it when |
|---|---|
| **mission load** | from accepting the mission until you have control |
| **walking** | walking around, no enemies in range |
| **boost** | boosting (ground and air) |
| **jump** | jumping / flying |
| **lock on** | locking onto an enemy |
| **fire weapon** | firing the right-arm weapon |
| **fire back weapon** | firing shoulder / back weapons (missiles, cannons) |
| **blade** | using a laser blade |
| **switch weapon** | changing the selected weapon |
| **taking damage** | being hit |
| **enemy destroyed** | the moment an enemy dies (type, then destroy one) |
| **radio** | a radio message / voice line |
| **pause menu** | the in-mission pause menu |
| **leaving area** | at the area border (LEAVING AREA warning) |
| **objective done** | when an objective completes |
| **mission complete** | mission end, until the results screen |
| **results** | results, pay and repair or ammo costs |

### 3. Missions: the rest

| label | type it when |
|---|---|
| **low hp** | the damage warning |
| **death** | AC destroyed, until the game over or retry screen |
| **mission failed** | failing by an objective (timer out, target lost) |
| **retry** | retrying a failed mission |
| **boss** | a boss or special enemy fight |
| **defend** | a defend / escort objective in progress |

Also play as many **different missions** as you can. Each mission has its own overlay of
30–70 functions, so every new mission adds code that no other mission runs. There are
49 story-mission overlays (mission N is overlay `fdat` 2N, e.g. mission 30 =
`fdat060`). Missions not unlocked yet can be reached two ways:

- from the archived states in `disc_map/states/STATES.tsv`
  (`python tools/duckstation/states.py list`; `states.py stage <id> <slot>` copies one
  into a DuckStation slot): missions 20, 24, 28, 30 and 34 and the garage;
- by asking Claude to force a mission from the garage (it pokes the mission number before
  launch).

### 4. Versus modes

| label | type it when |
|---|---|
| **vs menu** | versus mode menus and AC choice |
| **vs fight** | a split-screen fight (needs a second pad, or a pad bound to the keyboard) |
| **vs results** | the end of a versus fight |

Link-cable versus (mode 1) needs two consoles. Skip it.

## Arguments and return values (for the unclear tier)

Once the naming pass has a list of unclear functions, a capture run records what goes in
and out of each one. For example, a function that returns 412 right after you equip a
part weighing 412 is a weight getter.

```
python tools/duckstation/coverage_tag.py --context assembly --capture ac_calc_performance --capture ovl201:0x80075898
```

- `--capture` takes `unit:address`, an exe address, or a name from `re/match/hints.tsv`.
- Each call records a0..a3 on entry, and v0/v1 on return.
- For each argument that points into RAM, it also records 16 bytes behind it, both on
  entry and on return, so output structs show up.
- Up to 5 calls per function per label; `--captures N` changes that.
- Rows go to `re/match/calls.tsv`, and captured functions count for coverage as usual.

## What a session leaves behind

- `re/match/coverage.tsv`: `unit vaddr context hits`, merged with earlier runs.
- `re/match/hints.tsv`: one `play coverage: …` hint per function hit, which the match
  viewer and the naming pass read.
- `re/match/calls.tsv` (capture runs): one row per recorded call.

The script never commits. After a session, tell Claude. It commits the three files, and
the naming pass picks them up from there.

## Later sessions

`--set unseen` arms only functions with no coverage row yet. That makes the game much
lighter once the common paths are covered, and keeps every breakpoint on new code.
`--set unnamed` arms only functions that still have no name.
