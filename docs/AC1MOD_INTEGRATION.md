# AC1mod — companion project pointer

AC1mod is a **separate companion viewer project** (its own repo): a PyQt disc
browser + PS1 stage/model viewer that consumes this repository's PA##.T geometry
work. The files it references (`main.py`, `core/pa_parser.py`, `core/jpsxdec.py`,
its 3D view widget, etc.) live in that companion repo, not here.

The integration point with this repo: the companion viewer's geometry decoder is a
port of this repo's authoritative PA decode in `tools/pa/pa_obj.py` /
`tools/pa/pa_parse.py` — keep a single source of truth here and vendor/import it
there, do not fork the logic. See:

- [PA_FORMAT.md](PA_FORMAT.md) — the PA##.T container + primitive format (the decode being ported).
- [PA_HEADER.md](PA_HEADER.md) / [PA_SLOTS.md](PA_SLOTS.md) — header and slot structure.

Verification of the decode itself stays in this repo: `python3 tools/pa/pa_obj.py …`
produces a mesh whose face indices are all valid and whose shape is recognizable in
any OBJ viewer.
