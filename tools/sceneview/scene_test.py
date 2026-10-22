#!/usr/bin/env python3
"""
scene_test.py — tests for tools/sceneview/scene.py on SYNTHETIC data.

No disc or save state is needed: the test builds one PA geometry block by hand (a
textured quad in the base LOD and a flat triangle in the detail range, laid out as
docs/PA_FORMAT.md describes), places it twice through a stubbed placement table,
and checks the decoded UVs, materials, LOD flags, quad split, world placement and
de-duplication. It also checks the GPU-VRAM marker lookup and the save-state name
parser. The byte values are arbitrary test inputs, not RE findings. Exit code is
nonzero on any failure.
"""
import os, struct, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import scene             # noqa: E402
import savestate as SS   # noqa: E402  (scene puts tools/duckstation on sys.path)

FAILS = []


def check(cond, msg):
    print(("ok   " if cond else "FAIL ") + msg)
    if not cond:
        FAILS.append(msg)


def fake_block():
    """One sub-object: 4 verts; record 0 = textured quad 0x2c (base LOD), record 1 =
    flat tri 0x20 (detail range: base prim count 1, FLAGS low bits = 1)."""
    O = 0x0c                       # block[+8]: section base; reloc base = O + 12
    reloc = O + 12
    quad = bytearray(32)
    quad[0], quad[1], quad[3] = 11, 7, 0x2c
    for k, (u, v) in enumerate(((0, 0), (63, 0), (0, 31), (63, 31))):
        off = (0x04, 0x08, 0x0c, 0x10)[k]
        quad[off], quad[off + 1] = u, v
    struct.pack_into("<H", quad, 0x06, 0x78c0)          # CLUT word
    struct.pack_into("<H", quad, 0x0a, 0x0087)          # TPAGE word
    struct.pack_into("<4H", quad, 0x16, 0, 1, 2, 3)     # vertex indices
    tri = bytearray(16)
    tri[0], tri[1], tri[3] = 9, 3, 0x20
    tri[4:7] = bytes((200, 100, 50))
    struct.pack_into("<3H", tri, 0x0a, 0, 2, 1)
    verts = b"".join(struct.pack("<4h", *v) for v in
                     ((0, 0, 0, 0), (400, 0, 0, 0), (0, 0, 400, 0), (400, 0, 400, 0)))
    prim_abs = reloc + 28
    vtx_abs = prim_abs + len(quad) + len(tri)
    desc = bytearray(28)
    struct.pack_into("<I", desc, 0x00, vtx_abs - reloc)
    struct.pack_into("<HH", desc, 0x04, 4, 0)
    struct.pack_into("<I", desc, 0x08, vtx_abs + len(verts) - reloc)
    struct.pack_into("<H", desc, 0x0e, 1)               # 1 detail-LOD record
    struct.pack_into("<I", desc, 0x10, prim_abs - reloc)
    struct.pack_into("<H", desc, 0x14, 1)               # 1 base-LOD record
    body = bytearray(O + 12)
    struct.pack_into("<I", body, O + 8, 1)              # sub-object count
    blk = body + desc + quad + tri + verts
    struct.pack_into("<I", blk, 0, len(blk))
    struct.pack_into("<I", blk, 8, O)
    return bytes(blk)


def main() -> int:
    blk = fake_block()
    faces = scene.block_faces(blk)
    check(len(faces) == 3, f"quad + tri decode to 3 triangles (got {len(faces)})")
    (q0, q1, t0) = faces
    check(q0[1] == [(0, 0), (63, 0), (0, 31)] and q1[1] == [(63, 0), (63, 31), (0, 31)],
          "quad UVs follow the PSX Z/N split (0,1,2)+(1,3,2)")
    check(q0[2] == 0x0087 and q0[3] == 0x78c0, "textured face keeps its TPAGE and CLUT words")
    check(q0[4] == scene.NEUTRAL and q0[6], "GTE-lit textured face modulates at 0x80")
    check(not q0[5] and t0[5], "base-LOD quad vs detail-range triangle")
    check(t0[1] is None and t0[4] == (200, 100, 50) and not t0[6], "flat face keeps its colour, no UVs")

    real = (scene.MG.geom_blocks, scene.MG.placements)
    try:
        scene.MG.geom_blocks = lambda stream: [blk]
        scene.MG.placements = lambda stream, n: [(0, (1000, -50, 0), 0), (0, (1000, -50, 0), 0)]
        s = scene.level_scene(b"")
        check(s["tris"] == 3, f"second instance at the same place is de-duplicated (tris {s['tris']})")
        check(s["pos"][3:6] == [400 * 0.25 + 1000, -50, 0], "world = local * SCALE + translation")
        check(s["uv"][:6] == [0, 0, 63, 0, 0, 31], "flat uv list, 6 per triangle")
        check([f & 3 for f in s["flags"]] == [1, 1, 2], "flags: bit0 textured, bit1 detail-only")
        check(len(s["sections"]) == 2 and s["sections"][0]["bbox"][0] == 1000, "per-section bbox")
        s2 = scene.level_scene(b"", dedupe=False)
        check(s2["tris"] == 6, "dedupe=False keeps both instances")
    finally:
        scene.MG.geom_blocks, scene.MG.placements = real

    vram = bytes(range(256)) * (SS.VRAM_SIZE // 256)
    blob = b"\x03\0\0\0GPU" + b"\xaa" * 0x2b0 + SS.VRAM_MARKER + vram + b"\x0f\0\0\0GPUTextureCache"
    off = SS.vram_offset(blob)
    check(off == 7 + 0x2b0 + len(SS.VRAM_MARKER) and blob[off:off + SS.VRAM_SIZE] == vram,
          "VRAM is the 1 MB right after the GPU-VRAM marker")
    check(scene.vram_from_blob(blob) == vram, "scene.vram_from_blob uses the marker")
    check(SS.vram_offset(b"\x03\0\0\0GPU" + vram) is None, "no marker -> None")

    with tempfile.TemporaryDirectory() as d:
        for f in ("M30-mid-01__20261018__v87.sav", "GAR-main-01__20261017__v87.sav",
                  "M30-mid-01__20261018__v87.json", "notes.txt"):
            open(os.path.join(d, f), "w").close()
        st = scene.list_states(d)
        check([(x["id"], x["mission"]) for x in st] == [("GAR-main-01", None), ("M30-mid-01", 30)],
              "library .sav names parse to id + mission")

    print(f"\n{len(FAILS)} failure(s)")
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
