#!/usr/bin/env python3
"""
browser-tools/sceneview/scene.py — scene data for the browser scene viewer (server.py).

Decodes a mission's walkable level the way tools/ac1mod/core/level.py does
(FDAT entry 2N+1: chunk-0 PA blocks, chunk-7 translation-only placement, world =
local * SCALE + t; docs/LEVEL_LOADING.md), but keeps what the viewer needs and
level.py drops:

  * per-corner texture coordinates (u, v bytes), TPAGE and CLUT words of every
    textured face (docs/PA_FORMAT.md "Textured-primitive MATERIAL words");
  * whether a face is in the sub-object's base LOD or only in its detail range
    (record index >= u16@+0x14, docs/PA_FORMAT.md "Two LODs");
  * the modulation colour of unlit faces (rec+0x10 / +0x14) and the flat colour of
    untextured ones.

And the texels: a DuckStation save state's 1 MB VRAM (docs/SAVESTATE_FORMAT.md),
which the page samples like the PS1 GPU does (4/8/15 bpp through the CLUT).

Coordinates stay PSX (Y down); the page flips Y.  Nothing here is written to disk:
the server streams it to 127.0.0.1 only (the geometry and texels are game data).
"""
from __future__ import annotations
import os, re, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
for p in ("tools/pa", "tools/mission", "tools/duckstation", "tools/ac1mod"):
    p = os.path.join(REPO, *p.split("/"))
    if p not in sys.path:
        sys.path.insert(0, p)

import pa_obj                     # noqa: E402  PRIM_VERTS, TEXTURED, parse_subobjects, read_verts
import mission_geom as MG         # noqa: E402  geom_blocks, placements, SCALE

HORIZ = 0.7                       # |ny| above this = floor/ceiling (as core/level.py)
CLASS = {"floor": 0, "wall": 1, "ceiling": 2}
NEUTRAL = (128, 128, 128)         # PS1 texture modulation 0x80 = x1.0 (GTE-lit faces)

VRAM_SIZE = 1024 * 512 * 2


# ------------------------------------------------------------------ geometry ----
def walk_records(block, s):
    """Every primitive record of sub-object `s`: dicts with type, vertex indices,
    uv [(u,v)] (textured only), tpage, clut, rgb and detail (False = base LOD)."""
    out = []
    o = s["prim_off"]
    for n in range(s["prim_cnt"]):
        if o + 4 > len(block):
            break
        reclen = 4 + block[o + 1] * 4
        typ = block[o + 3] & 0xbc
        if o + reclen > len(block):
            break
        info = pa_obj.PRIM_VERTS.get(typ)
        if info:
            voff, nv, stride = info
            idx = [struct.unpack_from("<H", block, o + voff + stride * k)[0]
                   for k in range(nv) if o + voff + stride * k + 2 <= len(block)]
            rec = dict(type=typ, idx=idx, detail=n >= s["prim_base_cnt"],
                       uv=None, tpage=0, clut=0, rgb=None)
            if pa_obj.TEXTURED(typ) and o + 0x0c + 4 * (nv - 3) + 2 <= len(block):
                rec["uv"] = [(block[o + 4 + 4 * k], block[o + 5 + 4 * k]) for k in range(nv)]
                rec["clut"] = struct.unpack_from("<H", block, o + 0x06)[0]
                rec["tpage"] = struct.unpack_from("<H", block, o + 0x0a)[0]
                col = {0xa4: 0x10, 0xac: 0x14}.get(typ)
                rec["rgb"] = tuple(block[o + col:o + col + 3]) if col and o + col + 3 <= len(block) else NEUTRAL
            elif o + 7 <= len(block):
                rec["rgb"] = tuple(block[o + 4:o + 7])
            out.append(rec)
        o += reclen
    return out


def face_class(a, b, c):
    """floor / wall / ceiling by world normal; PSX Y is down, a floor's winding
    normal cross(b-a, c-a) points +Y (docs/LEVEL_LOADING.md §5 item 4)."""
    ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
    vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
    nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
    m = (nx * nx + ny * ny + nz * nz) ** 0.5 or 1.0
    ny /= m
    if abs(ny) > HORIZ:
        return "floor" if ny > 0 else "ceiling"
    return "wall"


def block_faces(block):
    """Triangles of one PA block in local coords: (verts3, uv3|None, tpage, clut,
    rgb, detail, textured). Quads split in PSX Z/N order (0,1,2)+(1,3,2)."""
    _, subs = pa_obj.parse_subobjects(block)
    tris = []
    for s in subs:
        verts = pa_obj.read_verts(block, s["vtx_off"], s["vtx_cnt"])
        nv = len(verts)
        for r in walk_records(block, s):
            idx = r["idx"]
            if len(idx) < 3 or any(i >= nv for i in idx) or len(set(idx)) < len(idx):
                continue
            uv = r["uv"]
            orders = [(0, 1, 2)] if len(idx) == 3 else [(0, 1, 2), (1, 3, 2)]
            for t in orders:
                tris.append(([verts[idx[k]][:3] for k in t],
                             [uv[k] for k in t] if uv else None,
                             r["tpage"], r["clut"], r["rgb"] or NEUTRAL, r["detail"],
                             uv is not None))
    return tris


def mission_stream(bin_path, n, index_path=None):
    from core.fdat import mission_entry          # tools/ac1mod/core
    return mission_entry(bin_path, n, odd=True, index_path=index_path)


def level_scene(stream, dedupe=True):
    """The viewer's scene for one mission chunk stream (dict of flat lists).

    pos   9 floats per tri (world, PSX Y down)     uv    6 bytes per tri (0 if untextured)
    tpage/clut  1 each per tri                     rgb   3 per tri
    flags 1 per tri: bit0 textured, bit1 detail-LOD only, bits2-3 class
    sec   1 per tri: placed-section index; sections: [{block, center, bbox}]"""
    blocks = MG.geom_blocks(stream)
    plc = MG.placements(stream, len(blocks))
    S = MG.SCALE
    pos, uvs, tpage, clut, rgb, flags, sec = [], [], [], [], [], [], []
    sections, cache, seen = [], {}, set()
    for si, (blk, (tx, ty, tz), _light) in enumerate(plc):
        if blk not in cache:
            cache[blk] = block_faces(blocks[blk])
        lo, hi = [1e18] * 3, [-1e18] * 3
        for (vs, uv, tp, cl, col, detail, textured) in cache[blk]:
            w = [(x * S + tx, y * S + ty, z * S + tz) for (x, y, z) in vs]
            if dedupe:
                # sections instanced edge to edge author shared faces twice with the
                # same winding (core/level.py _dedupe); keep one per (place, winding)
                tri = tuple(tuple(round(c * 4) for c in p) for p in w)
                key = min((tri[0], tri[1], tri[2]), (tri[1], tri[2], tri[0]), (tri[2], tri[0], tri[1]))
                if key in seen:
                    continue
                seen.add(key)
            for p in w:
                for k in range(3):
                    lo[k] = min(lo[k], p[k]); hi[k] = max(hi[k], p[k])
                pos.extend(p)
            uvs.extend(c for pt in (uv or [(0, 0)] * 3) for c in pt)
            tpage.append(tp); clut.append(cl); rgb.extend(col)
            flags.append((1 if textured else 0) | (2 if detail else 0)
                         | (CLASS[face_class(*w)] << 2))
            sec.append(si)
        if lo[0] > hi[0]:
            lo = hi = [tx, ty, tz]
        sections.append(dict(block=blk, bbox=[*lo, *hi],
                             center=[(lo[k] + hi[k]) / 2 for k in range(3)]))
    return dict(pos=pos, uv=uvs, tpage=tpage, clut=clut, rgb=rgb, flags=flags,
                sec=sec, sections=sections, tris=len(flags))


# --------------------------------------------------------------------- VRAM -----
def vram_from_blob(blob):
    """The 1 MB VRAM of a decompressed DuckStation state (1024x512 16bpp), located by
    its "GPU-VRAM" marker (savestate.vram_offset)."""
    import savestate as SS
    o = SS.vram_offset(blob)
    if o is None:
        raise ValueError("no GPU-VRAM marker in this state")
    return blob[o: o + VRAM_SIZE]


# Level texels on disc: GG/COM/RTIM.T. The mission loader reads RTIM entry 100 (common)
# then entry missionNum into VRAM (docs/MISSION_SYSTEM.md). Each entry is a run of
# standard TIMs, each block carrying its own VRAM rect. Verified 2026-10-22: RTIM 100 + 30
# rebuild mission 30's level pages (x 320..640, tpage 0x85..0x89) and their CLUT rows
# 480..485 byte for byte against M30-mid-01's VRAM; what differs (the page at x 640, CLUT
# rows 486+) is object/AC art the game uploads later from elsewhere.
RTIM_PATH = "GG/COM/RTIM.T"
RTIM_COMMON = 100


def tim_rects(entry):
    """[(x, y, w, h, data)] of every TIM block (CLUT, then image) in an RTIM entry, in
    upload order; w is in 16-bit VRAM pixels. Stops at the first non-TIM word."""
    o, out = 0, []
    while o + 8 <= len(entry):
        tid, flags = struct.unpack_from("<II", entry, o)
        if tid != 0x10:
            break
        p = o + 8
        for _ in range(2 if flags & 8 else 1):
            if p + 12 > len(entry):
                return out
            ln, x, y, w, h = struct.unpack_from("<IHHHH", entry, p)
            if ln < 12 or p + ln > len(entry):
                return out
            out.append((x, y, w, h, entry[p + 12:p + 12 + w * h * 2]))
            p += ln
        o = p
    return out


def upload(vram, rects):
    """Copy TIM blocks into a bytearray VRAM like LoadImage (clipped to 1024x512)."""
    for (x, y, w, h, data) in rects:
        for r in range(h):
            if y + r >= 512:
                break
            row = data[r * w * 2:(r + 1) * w * 2][:(1024 - x) * 2]
            o = ((y + r) * 1024 + x) * 2
            vram[o:o + len(row)] = row


def rtim_entries():
    return pa_obj.load_container(RTIM_PATH)     # reads the disc from $AC1_BIN


def vram_from_rtim(n, entries=None):
    """VRAM as mission N's load leaves the level textures: RTIM 100, then RTIM N."""
    ents = entries or rtim_entries()
    vram = bytearray(VRAM_SIZE)
    for i in (RTIM_COMMON, n):
        if 0 <= i < len(ents) and ents[i]:
            upload(vram, tim_rects(ents[i]))
    return bytes(vram)


def states_dir():
    return os.environ.get("AC1_STATES_DIR") or os.path.join(os.path.expanduser("~"), "dev", "ac1_states")


STATE_RE = re.compile(r"^(?P<id>(?P<ctx>[A-Z]+)(?P<num>\d+)?-[a-z]+-\d+)__\d{8}__v\d+\.sav$")


def list_states(lib=None):
    """[{id, file, mission}] of the library's states (mission None for GAR-* etc.)."""
    lib = lib or states_dir()
    out = []
    if os.path.isdir(lib):
        for f in sorted(os.listdir(lib)):
            m = STATE_RE.match(f)
            if m:
                num = m.group("num")
                out.append(dict(id=m.group("id"), file=os.path.join(lib, f),
                                mission=int(num) if m.group("ctx") == "M" and num else None))
    return out


def state_info(path, disc_bin):
    """(vram bytes, player dict) of a save state. Player slot 0 is 0x801A26B8:
    pos int16 +0x08/+0x0A/+0x0C (feet), +0x12 yaw (4096 = 360 deg), collision
    half-width +0x28 and height +0x2A (docs/PLAYER_CONTROL.md §4)."""
    import savestate as SS
    st = SS.SaveState(path, disc_bin=disc_bin)
    s16 = lambda a: (st.u16(a) ^ 0x8000) - 0x8000
    s0 = 0x801A26B8
    player = dict(pos=[s16(s0 + 0x08), s16(s0 + 0x0a), s16(s0 + 0x0c)],
                  yaw=s16(s0 + 0x12), half_width=st.u16(s0 + 0x28), height=st.u16(s0 + 0x2a),
                  mission=st.mission_byte())
    return vram_from_blob(st.blob), player
