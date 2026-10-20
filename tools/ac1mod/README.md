# AC1mod (`tools/ac1mod/`)

Desktop tool for browsing, previewing, annotating and modding the assets of
*Armored Core* (`SLUS-01323`): a PyQt6 disc browser and 3D stage/model/mission
viewer built on [jPSXdec](https://github.com/m35/jpsxdec), plus a headless CLI.

It was a separate repo ([byroweb/AC1mod](https://github.com/byroweb/AC1mod),
imported here at `616bcd2`). Its history stays in that repo; new work happens here.

> **No game data is included.** You supply your own disc image. Everything the tool
> writes (extracted files, the jPSXdec index, `.ac1mod` projects, renders) goes into
> a per-BIN folder next to the app, which `.gitignore` excludes.

## Features
- **Disc browser.** Shows the jPSXdec file index and previews TIM images, text,
  hex and XA audio per entry. You can import replacement images and audio.
- **PA##.T model viewer.** Decodes the stage/object geometry packs into an
  orbitable mesh, with flat, wireframe or material-coloured shading.
- **Missions.** Assembles a mission's walkable level and overlays its spawns
  (`core/level.py`, `core/mission.py`). Also extracts the MIS.T preview TIMs
  (`core/mis.py`).
- **Memory cards.** Browses `.mcd` cards and imports or exports AC1 emblems
  (`core/memcard.py`, [docs/AC1_EMBLEM.md](../../docs/AC1_EMBLEM.md)).
- **Notes.** Per-PA annotations are saved in the `.ac1mod` project file.
- **Headless CLI** (`ac1mod_cli.py`): `list / info / note / notes / obj / render /
  level / missions / mistim / memcard / emblem-write-mcs`.

## Requirements
- Python 3.10+ with `requirements.txt` (PyQt6, Pillow, numpy, pygame).
- Java 21 and jPSXdec v2.1. The jar is found in this order: `$JPSXDEC_JAR`, then
  `tools/ac1mod/jpsxdec/jpsxdec.jar`, then `~/dev/jpsxdec/*/jpsxdec.jar`, then
  `~/Applications/jpsxdec/*/`. Java is taken from `$JAVA_HOME/bin`, else `PATH`.
  This is the same lookup as `tools/audio/` (see [tools/README.md](../README.md) § jPSXdec).
- Optional: a host C compiler (`$CC`, `gcc` or `clang`). The rasterizer
  (`core/raster.c`) and XA decoder (`core/xa.c`) compile on first use into
  `core/lib*.dll` (Windows) or `core/lib*.so`. Without a compiler the tool uses
  the slower numpy rasterizer and `tools/audio/xa_adpcm.py`'s Python decoder, with
  the same output.

## Run
From Git Bash on Windows, or a shell on Linux:
```sh
tools/ac1mod/setup.sh          # creates tools/ac1mod/.venv and runs a health check
tools/ac1mod/run_ac1mod.sh     # GUI
```
CLI (`PY` = `tools/ac1mod/.venv/Scripts/python.exe` on Windows, `.venv/bin/python` on Linux):
```sh
$PY tools/ac1mod/ac1mod_cli.py --project P.ac1mod list
$PY tools/ac1mod/ac1mod_cli.py --project P.ac1mod render GG/P0/PA00.T --entry 2 -o pa00_e2.png
$PY tools/ac1mod/ac1mod_cli.py --project P.ac1mod level 1 --materials --spawns -o m01.png
```
Without `--project`, the CLI uses `tools/ac1mod/recent_project.txt` (written by the
GUI), or the single `*.ac1mod` file in `tools/ac1mod/` if there is exactly one.

## Relation to the rest of `tools/`
AC1mod imports the repo's format decoders instead of carrying its own:
PA geometry from `tools/pa/pa_obj.py`, `.T` containers from
`tools/extract/{extract_t,t_repack}.py`, XA-ADPCM from
`tools/audio/xa_adpcm.py` and the mission chunk stream (chunk-0 blocks, chunk-7
placement, chunk-12 spawns) from `tools/mission/{mission_geom,mission_parse}.py`
(`core/_tools.py` puts those directories on `sys.path`). `core/pa_parser.py`,
`core/fdat.py`, `core/mis.py`, `core/level.py`, `core/mission.py` and `core/xa.py`
only adapt their output (Mesh objects, material colours, FDAT mission slots, level
face classing and dedupe, spawn markers, the C fast path in `core/xa.c`, which is a bit-identical port of
`xa_adpcm.decode_stream_float`). A format fix goes into `tools/`. See
[docs/AC1MOD_INTEGRATION.md](../../docs/AC1MOD_INTEGRATION.md).

Regression check for the PA decode: `info GG/P0/PA00.T` should report entry 2 =
180 verts / 323 faces.
