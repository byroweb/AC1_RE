"""
core/_tools.py — make the repo's authoritative format decoders importable.

AC1mod lives in tools/ac1mod/ and owns no format logic of its own: PA geometry
comes from tools/pa/pa_obj.py, .T containers from tools/extract/{extract_t,
t_repack}.py, XA-ADPCM from tools/audio/xa_adpcm.py and the mission chunk stream
(chunk walk, chunk-0 blocks, chunk-7 placement, chunk-12 spawns) from
tools/mission/{mission_parse,mission_geom}.py. Import this module, then
import those by their plain names. The directories are appended (not prepended)
so they can never shadow AC1mod's own modules or installed packages.
"""
from __future__ import annotations
import sys
from pathlib import Path

TOOLS = Path(__file__).resolve().parents[2]

for _d in ("pa", "extract", "audio", "mission"):
    _p = str(TOOLS / _d)
    if _p not in sys.path:
        sys.path.append(_p)
