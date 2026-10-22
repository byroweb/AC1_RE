# browser-tools/ — interactive explorers for the AC1 project

Small local web apps for *looking at* and *turning the knobs on* whatever the project is
working on: the mission loop, the matching machinery, a level. Each one is a tiny Python
server plus one page, opened in a browser. They don't have to look alike, but they share
how they're built, started and found.

**Start here:** double-click `browser-tools\launch.cmd` (or the **AC1 Browser Tools**
desktop shortcut), or run

```bash
python browser-tools/launchpad/server.py --open      # http://127.0.0.1:8760/
```

The launchpad lists every tool with what it shows and what it needs, and whether it is
running. It starts any of them (with a variant, e.g. plantview's live vs. save-state
source) and opens the page. Tools started from the launchpad stop when it does; their
output is logged in `%TEMP%\ac1-browser-tools\<tool>.log`.

## The tools

| tool | port | what you can explore | needs |
|---|---|---|---|
| [`plantview/`](plantview/README.md) | 8765 | the mission loop as a factory floor: lifecycle, entity table, per-frame pipeline | save states or DuckStation (synthetic needs nothing) |
| [`match-viewer/`](../tools/loop/README.md#viewerpy-watch-a-run-leave-hints) | 8767 | an overnight matching pass: target asm / C / our asm, hints, triage, Try buttons | an `overnight.sh` work dir |
| [`sceneview/`](sceneview/README.md) | 8771 | any level drawn the PS1 way with its real textures; pilot, camera, texture and PS1 knobs | disc image (`$AC1_BIN`) |
| [`launchpad/`](launchpad/server.py) | 8760 | this list | nothing |

Ports are a registry: take the next free one (8780, 8790, ...) when you add a tool.

## Anatomy of a tool

```
browser-tools/<name>/
  tool.json      manifest the launchpad reads (below)
  server.py      stdlib HTTP server on 127.0.0.1 (the older tools name it poller.py / viewer.py)
  index.html     one self-contained page: inline CSS/JS, no build step, no CDN
  <name>.cmd     double-click launcher: cd to the repo root, run the server with --open
  README.md      what it shows, where each number comes from, the knobs
  *_test.py      tests for its parsers / data path (synthetic data, no disc needed)
```

`tool.json`:

```json
{
  "name": "Scene viewer",
  "summary": "One sentence: what you can explore and turn.",
  "order": 30,
  "port": 8771,
  "start": ["server.py", "--port", "{port}"],
  "variants": [{"label": "default", "args": []}],
  "needs": ["disc image ($AC1_BIN)"],
  "launcher": "sceneview.cmd",
  "readme": "README.md"
}
```

`start[0]` is relative to the tool's folder and runs with the launchpad's Python, from
the repo root. `{port}` is substituted. Each variant's `args` are appended.

## House rules

- **127.0.0.1 only, and nothing committed from the game.** Tools stream game data (disc
  contents, save-state RAM/VRAM) to the local page; they never bind other interfaces and
  never write that data into the repo.
- **Stdlib Python, one page.** No frameworks and no CDN, so a tool runs on a fresh PC with just
  Python. The shared helper `_shared/btserve.py` gives you the `Handler` (JSON / bytes /
  file responses, quiet logging, no-store caching) and `run()` (127.0.0.1, `--open`, and
  "already running → just open the page", which makes double-clicking always safe).
- **`--open` and a `.cmd` launcher.** Every tool starts with one double-click.
- **Light and dark.** Colours are CSS custom properties on `:root`, redefined for dark under
  `prefers-color-scheme` and for a `data-theme` override. Include a ◐ toggle.
- **Works at phone width** (the panel stacks under the view), so it's usable from another
  device over Remote Control.
- **Say where the numbers come from.** The README names the docs and addresses behind each
  view, and marks what is decoded vs. assumed (e.g. sceneview's "AC height = 10 m").
- **Knobs first.** Sliders, toggles and presets for the questions the tool exists to
  answer, plus a screenshot or export button when a picture is the deliverable.

## Adding a tool

1. `cp -r browser-tools/_template browser-tools/<name>`, rename `tool.cmd`.
2. Pick a port from the registry above; set it in `server.py` and `tool.json`.
3. Replace `/api/data` and the page's draw code; keep the head/tokens/layout.
4. Add a row to the table above, a test, and a README. The launchpad picks it up from `tool.json`.

`_shared/btserve_test.py` checks every `tool.json` (required keys, unique ports, the start
script, launcher and README exist) and the helper's serve/reuse behaviour.
