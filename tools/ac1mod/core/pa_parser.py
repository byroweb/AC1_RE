"""
core/pa_parser.py — AC1mod's mesh view of PA##.T stage/object geometry.

A thin adapter: the PA decode itself (sub-object table, vertex pools, primitive
records, flat RGB / textured (tpage, clut) shading, Z/N quad split) is
tools/pa/pa_obj.py, and .T container splitting is tools/extract/t_repack.py —
see docs/PA_FORMAT.md. This module only turns their output into AC1mod's Mesh /
Face objects, colours textured faces with material_color, and lays blocks out
for viewing (contact sheet, world-space stage).
"""
from __future__ import annotations
import math
import struct
from dataclasses import dataclass, field

from core import _tools  # noqa: F401  (puts tools/pa, tools/extract on sys.path)
import extract_t
import pa_obj
import t_repack

FLAT_GREY = (170, 170, 170)    # shading words missing (truncated record)


def material_color(tpage: int, clut: int) -> tuple:
    """Stable, distinct RGB for a (tpage,clut) texture material.

    The stage texels have not been located yet (docs/PA_FORMAT.md, "Textured-
    primitive MATERIAL words"; docs/AC1MOD_INTEGRATION.md §1b). Until they are, or
    a live VRAM dump is supplied, colour each textured face by its decoded material
    id so distinct textures read as distinct flat colours (a "material map")."""
    import colorsys
    key = ((tpage & 0xffff) * 0x9E3779B1 ^ (clut & 0xffff) * 0x85EBCA77) & 0xffffffff
    h = (key & 0xff) / 255.0
    s = 0.45 + ((key >> 8) & 0x3f) / 63.0 * 0.35
    v = 0.62 + ((key >> 14) & 0x3f) / 63.0 * 0.33
    r, g, b = colorsys.hsv_to_rgb(h, s, v)
    return (int(r * 255), int(g * 255), int(b * 255))


@dataclass
class Face:
    verts: tuple              # global indices into Mesh.vertices
    color: tuple = FLAT_GREY
    textured: bool = False
    mat: tuple = None         # (tpage, clut) for textured faces, else None


@dataclass
class Mesh:
    vertices: list = field(default_factory=list)   # [(x,y,z)]
    faces: list = field(default_factory=list)      # [Face]
    groups: list = field(default_factory=list)     # [(label, vstart, vcount)]

    def bbox(self):
        if not self.vertices:
            return None
        xs = [v[0] for v in self.vertices]; ys = [v[1] for v in self.vertices]
        zs = [v[2] for v in self.vertices]
        return (min(xs), min(ys), min(zs), max(xs), max(ys), max(zs))

    def stats(self):
        return dict(verts=len(self.vertices), faces=len(self.faces),
                    groups=len(self.groups))


# ---------------------------------------------------------------- container ----

def read_container(bin_path, sector_start, sector_end):
    """A .T file's entry payloads, read from its sectors in the disc image."""
    flat = extract_t.read_sectors(bin_path, sector_start, sector_end - sector_start + 1)
    return t_repack.unpack(flat).entries


def is_geometry_block(entry: bytes) -> bool:
    """Entries 2..N of a PA file are size-prefixed geometry blocks."""
    return len(entry) >= 12 and struct.unpack_from("<I", entry, 0)[0] == len(entry)


# ------------------------------------------------------------------ decode -----

def parse_block(block) -> Mesh:
    """Decode one geometry block into a Mesh (sub-objects concatenated)."""
    m = Mesh()
    _, subs = pa_obj.parse_subobjects(block)
    for s in subs:
        verts, tris, _ = pa_obj.subobject_tris(block, s)
        base = len(m.vertices)
        m.groups.append((f"sub{s['index']}", base, len(verts)))
        m.vertices.extend((x, y, z) for (x, y, z, _w) in verts)
        for (t, typ, rgb, mat) in tris:
            color = material_color(*mat) if mat else (rgb or FLAT_GREY)
            m.faces.append(Face(tuple(base + i for i in t), color,
                                pa_obj.TEXTURED(typ), mat))
    return m


def parse_pa_blocks(bin_path, sector_start, sector_end):
    """Return [(entry_index, Mesh)] for every geometry block in the PA file."""
    ents = read_container(bin_path, sector_start, sector_end)
    out = []
    for i, e in enumerate(ents):
        if is_geometry_block(e):
            try:
                mesh = parse_block(e)
            except Exception:
                continue
            if mesh.vertices:
                out.append((i, mesh))
    return out


def largest_block(blocks):
    """Pick the (entry_index, Mesh) with the most faces — the 'main' object."""
    return max(blocks, key=lambda b: len(b[1].faces), default=None)


def contact_sheet(bin_path, sector_start, sector_end, cell=100.0, gap=1.5) -> Mesh:
    """
    All geometry blocks as a NORMALIZED GRID: each block is centred and scaled so
    its largest dimension == `cell`, then placed in a square grid. A genuine
    contact sheet of every object in the PA file (small props and big stages show
    at comparable size), for figuring out *what* a PA file contains.
    """
    blocks = parse_pa_blocks(bin_path, sector_start, sector_end)
    out = Mesh()
    n = len(blocks)
    if not n:
        return out
    cols = max(1, math.ceil(math.sqrt(n)))
    step = cell * gap
    for k, (ei, m) in enumerate(blocks):
        bb = m.bbox()
        if not bb:
            continue
        cx, cy, cz = (bb[0]+bb[3])/2, (bb[1]+bb[4])/2, (bb[2]+bb[5])/2
        dim = max(bb[3]-bb[0], bb[4]-bb[1], bb[5]-bb[2], 1.0)
        s = cell / dim
        gx = (k % cols) * step
        gy = -(k // cols) * step
        base = len(out.vertices)
        out.groups.append((f"e{ei}", base, len(m.vertices)))
        out.vertices.extend(((v[0]-cx)*s + gx, (v[1]-cy)*s + gy, (v[2]-cz)*s)
                            for v in m.vertices)
        for fc in m.faces:
            out.faces.append(Face(tuple(base + i for i in fc.verts),
                                  fc.color, fc.textured))
    return out


def scene_mesh(bin_path, sector_start, sector_end, coord_limit=12000) -> Mesh:
    """
    Assemble a STAGE: merge all geometry blocks at their RAW (world) coordinates.

    AC1 authors a stage's environment geometry directly in world space, so merging
    the blocks unmodified reconstructs the actual level layout. Blocks whose bbox
    exceeds `coord_limit` are skipped — those are the object/effect slots (e125+,
    not-yet-cleanly-decoded sprites/MT models that share a local origin and would
    pile up at the centre); they're placed per-instance by mission data, not here.
    """
    out = Mesh()
    for ei, m in parse_pa_blocks(bin_path, sector_start, sector_end):
        bb = m.bbox()
        if not bb or max(abs(v) for v in bb) > coord_limit:
            continue
        base = len(out.vertices)
        out.groups.append((f"e{ei}", base, len(m.vertices)))
        out.vertices.extend(m.vertices)
        for f in m.faces:
            out.faces.append(Face(tuple(base + i for i in f.verts), f.color, f.textured))
    return out


# back-compat alias (old name)
combined_mesh = contact_sheet
