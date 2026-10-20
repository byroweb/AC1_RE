"""
core/mission.py — assemble a mission's populated 3D scene for AC1mod.

Reads mission N's spawn table (FDAT entry 2N+1, chunk 12: 256×40-byte records),
and builds a Scene of placed objects. Each spawn is a 3D MARKER at its world
position, coloured by its resource/class id; metadata (dispatch type, class id,
position, rotation, params) feeds the info dropdown. The marker mesh could later be
swapped for the spawn's geometry block (hw3 → block record, docs/MISSION_STAGES.md).

The spawn records are decoded by tools/mission/mission_parse.py (spawns: hw0-2 =
X,Y,Z in world units; hw3 = geometry block index AND dispatch type (-1 = none);
hw5 = rotation (4096 = 360°); hw7 = resource/class id; hw8..hw19 = params, hw8 =
defense argument, hw11 = HP; docs/OBJECT_STATS.md). This module builds the Scene.
"""
from __future__ import annotations
import math, colorsys
from core import _tools  # noqa: F401  (puts tools/mission on sys.path)
from core import pa_parser as PP
from core.scene import Scene, SceneObject
from core.fdat import mission_entry
import mission_parse

# Maps a spawn resource/class id (hw7) -> a human label; empty -> shown as
# "class N". Filled in as the class ids are named.
TYPE_LABELS: dict = {}

# Per-instance spawn params hw8..hw19 (index 0..11). There is no global per-type
# stat table; these in-record values carry the per-instance numbers
# (docs/OBJECT_STATS.md §1). hw11 = HP is CONFIRMED live; hw8 is the defense
# argument passed to the damage apply (its value curve is still open).
PARAM_LABELS = {3: "HP (hw11)", 4: "hw12 (range/area?)", 0: "defense (hw8)"}

MIS_SECTORS = (101920, 103449)   # MIS.T (mission text corpus)


def mission_names(bin_path, index_path=None):
    """{mission_number: name} from MIS.T entry 0 (0x20-byte stride name array)."""
    import re
    try:
        ents = PP.read_container(bin_path, *MIS_SECTORS)
    except Exception:
        return {}
    e0 = ents[0] if ents else b""
    names = {}
    for m in range(64):
        off = 0x100 + m * 0x20
        if off + 2 > len(e0):
            break
        end = e0.find(b">", off, off + 0x20)
        if end <= off:
            continue
        raw = e0[off:end]
        if all(0x20 <= c < 0x7f for c in raw):
            names[m] = raw.decode("ascii", "replace")
    return names


def spawns(bin_path, n, index_path=None):
    """Mission N's occupied spawn slots (mission_parse.spawns dicts)."""
    return mission_parse.spawns(mission_entry(bin_path, n, odd=True,
                                              index_path=index_path))


def type_color(t):
    r, g, b = colorsys.hsv_to_rgb((t * 0.137) % 1.0, 0.6, 1.0)
    return (int(r * 255), int(g * 255), int(b * 255))


def _marker_mesh(size=400.0):
    """A small upright diamond (octahedron) as a spawn marker."""
    s = size
    v = [(0, -s, 0), (s, 0, 0), (0, 0, s), (-s, 0, 0), (0, 0, -s), (0, s, 0)]
    f = [(0, 1, 2), (0, 2, 3), (0, 3, 4), (0, 4, 1),
         (5, 2, 1), (5, 3, 2), (5, 4, 3), (5, 1, 4)]
    m = PP.Mesh()
    m.vertices = [(float(a), float(b), float(c)) for a, b, c in v]
    m.faces = [PP.Face(tuple(t)) for t in f]
    return m


def mission_scene(bin_path, n, index_path=None, marker_size=None,
                  level=False, ceilings=True):
    """Build a Scene of typed markers at each spawn (info-only metadata attached).
    level=True also places the mission's ASSEMBLED walkable level (core/level.py,
    placement solved 2026-06-12); ceilings=False drops ceiling faces (the toggle)."""
    sc = Scene()
    if level:
        from core.level import level_mesh
        lm = level_mesh(bin_path, n, index_path, ceilings=ceilings)
        if lm.vertices:
            sc.add(SceneObject(name="level", mesh=lm,
                               meta={"sections": len(lm.groups),
                                     "ceilings": ceilings}))
    sp = spawns(bin_path, n, index_path)
    # Spawn X,Y,Z are already WORLD units, the same space as the placed level mesh
    # (only chunk-0 block geometry is authored at 4x; see mission_geom.SCALE).
    if marker_size is None and sp:
        xs = [s["x"] for s in sp]; zs = [s["z"] for s in sp]
        extent = max(max(xs) - min(xs), max(zs) - min(zs), 1000)
        marker_size = max(extent * 0.018, 120)     # ~2% of world span
    for s in sp:
        meta = {
            "type": s["typ"], "block": s["blk"],
            "pos": (s["x"], s["y"], s["z"]),
            "rot_deg": round(s["rot"] * 360 / 4096) % 360,
            "params": s["params"],
        }
        sc.add(SceneObject(
            name=f"#{s['i']} class{s['typ']}",
            mesh=_marker_mesh(marker_size),
            rot_y=s["rot"] * 2 * math.pi / 4096,
            trans=(s["x"], s["y"], s["z"]),
            color=type_color(s["typ"]),
            meta=meta,
        ))
    return sc, sp
