#!/usr/bin/env python3
"""Validate the gouraud 0x34/0x3c vertex-slot fix against raw PA files.

For every sub-object in every geometry block, walk the primitive stream and, for
each gouraud record (0x34 tri / 0x3c quad), decode the vertex indices BOTH ways:
  OLD: first=0x14, contiguous stride 2 (the tentative/buggy layout)
  NEW: 0x34=(0x12,3,4), 0x3c=(0x16,4,4) (live-RE'd interleaved layout)
and count how many produce an out-of-range index (idx >= vtx_cnt). A correct
layout should give ~0 OOR; the buggy one should give the 5-30% that were dropped.
"""
import sys, os, struct, collections
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools"))
import pa_obj

OLD = {0x34: (0x14, 3, 2), 0x3c: (0x14, 4, 2)}
NEW = {0x34: pa_obj.PRIM_VERTS[0x34], 0x3c: pa_obj.PRIM_VERTS[0x3c]}


def decode(block, o, voff, nv, stride):
    return [struct.unpack_from("<H", block, o + voff + stride * k)[0]
            for k in range(nv) if o + voff + stride * k + 2 <= len(block)]


def scan(path):
    ents = pa_obj.load_container(path)
    tot = collections.Counter()
    for ei, block in enumerate(ents):
        reloc, subs = pa_obj.parse_subobjects(block)
        for s in subs:
            vcnt = s["vtx_cnt"]
            o, n = s["prim_off"], 0
            while n < s["prim_cnt"] and o + 4 <= len(block):
                b1 = block[o + 1]
                typ = block[o + 3] & 0xbc
                reclen = 4 + b1 * 4
                if reclen < 4 or o + reclen > len(block):
                    break
                if typ in (0x34, 0x3c):
                    tot["gouraud_records"] += 1
                    for tag, table in (("old", OLD), ("new", NEW)):
                        voff, nv, st = table[typ]
                        idx = decode(block, o, voff, nv, st)
                        if any(i >= vcnt for i in idx) or len(idx) < nv:
                            tot[f"{tag}_OOR"] += 1
                o += reclen
                n += 1
    return tot


if __name__ == "__main__":
    paths = sys.argv[1:] or ["GG/P0/PA00.T", "GG/P1/PA20.T", "GG/P2/PA40.T"]
    grand = collections.Counter()
    for p in paths:
        t = scan(p)
        grand += t
        g = t["gouraud_records"]
        if g:
            print(f"{p}: gouraud={g}  OLD OOR={t['old_OOR']} "
                  f"({100*t['old_OOR']//g}%)  NEW OOR={t['new_OOR']} "
                  f"({100*t['new_OOR']//g}%)")
        else:
            print(f"{p}: no gouraud records")
    g = grand["gouraud_records"]
    if g:
        print(f"\nTOTAL: gouraud={g}  OLD OOR={grand['old_OOR']} "
              f"({100*grand['old_OOR']//g}%)  NEW OOR={grand['new_OOR']} "
              f"({100*grand['new_OOR']//g}%)")
