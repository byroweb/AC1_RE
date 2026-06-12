#!/usr/bin/env python3
"""
mission_map.py — top-down map of a mission's MT/object spawns.

Synthesis of two RE threads: the mission-runtime spawn table (docs/MISSION_SYSTEM.md,
chunk 12 = 256×40-byte records: hw0-2 = X,Y,Z, hw3 = geometry block index, hw5 = rot,
hw7 = object/MT type) rendered as a top-down (X horizontal, Z vertical) scatter so you
can see how a scene is populated — where enemies, the player start, and objectives sit.

Reads a mission CHUNK-STREAM entry (FDAT entry 2N+1, e.g. extracted NNN.bin). Renders
a PNG with one dot per spawn, coloured by type id, sized for visibility.

Usage:
  python3 tools/mission/mission_map.py disc_map/extracted/FDAT_T/entries/003.bin -o /tmp/m1.png
"""
import sys, os, struct, argparse, colorsys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mission_parse import walk_chunks, SPAWN_CHUNK
from PIL import Image, ImageDraw   # Pillow (AC1mod venv has it)

REC = 40


def type_color(t):
    h = (t * 0.137) % 1.0
    r, g, b = colorsys.hsv_to_rgb(h, 0.65, 1.0)
    return (int(r * 255), int(g * 255), int(b * 255))


def spawns(buf):
    sp = next((c for c in walk_chunks(buf) if c[0] == SPAWN_CHUNK), None)
    if not sp:
        return []
    _, off, ln, _ = sp
    out = []
    for i in range(ln // REC):
        hw = struct.unpack_from("<20h", buf, off + i * REC)
        x, y, z, blk, rot, typ = hw[2], hw[3], hw[4], hw[5], hw[7], hw[9]
        if (x, y, z, typ) == (0, 0, 0, 0) and blk == 0:
            continue                      # empty slot
        out.append(dict(i=i, x=x, y=y, z=z, blk=blk, rot=rot, typ=typ))
    return out


def render(recs, out, W=900, H=900, pad=40):
    img = Image.new("RGB", (W, H), (18, 20, 26))
    d = ImageDraw.Draw(img)
    if not recs:
        d.text((20, 20), "no spawns", fill=(200, 200, 200)); img.save(out); return
    xs = [r["x"] for r in recs]; zs = [r["z"] for r in recs]
    minx, maxx, minz, maxz = min(xs), max(xs), min(zs), max(zs)
    sx = (W - 2 * pad) / max(maxx - minx, 1)
    sz = (H - 2 * pad) / max(maxz - minz, 1)
    s = min(sx, sz)

    def px(x, z):
        return (pad + (x - minx) * s, pad + (z - minz) * s)
    # origin cross
    ox, oz = px(0, 0)
    d.line([(ox, pad), (ox, H - pad)], fill=(40, 44, 54)); d.line([(pad, oz), (W - pad, oz)], fill=(40, 44, 54))
    types = {}
    for r in recs:
        x, z = px(r["x"], r["z"]); col = type_color(r["typ"])
        # bigger dot for objects with a geometry block; ring for blk==-1 (logic/area)
        rad = 7 if r["blk"] >= 0 else 4
        d.ellipse([x - rad, z - rad, x + rad, z + rad], fill=col,
                  outline=(0, 0, 0) if r["blk"] >= 0 else col)
        d.text((x + rad + 1, z - 6), str(r["typ"]), fill=(210, 210, 215))
        types.setdefault(r["typ"], col)
    # legend
    d.text((10, 10), f"{len(recs)} spawns  |  X {minx}..{maxx}  Z {minz}..{maxz}"
           f"  |  dot=has geometry, ring=logic/area", fill=(180, 190, 200))
    ly = 30
    for t in sorted(types):
        d.rectangle([10, ly, 22, ly + 12], fill=types[t]); d.text((26, ly), f"type {t}", fill=(190, 190, 195))
        ly += 16
    img.save(out)
    return dict(n=len(recs), types=sorted(types), xr=(minx, maxx), zr=(minz, maxz))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stream", help="mission chunk-stream entry (NNN.bin = FDAT entry 2N+1)")
    ap.add_argument("-o", "--out", default="/tmp/mission_map.png")
    args = ap.parse_args()
    buf = open(args.stream, "rb").read()
    recs = spawns(buf)
    info = render(recs, args.out)
    print(f"wrote {args.out}: {info}")


if __name__ == "__main__":
    main()
