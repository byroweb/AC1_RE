#!/usr/bin/env python3
"""Parse the live block record table (0x8019F538) + instance/spawn table
(0x8019FAB8) dumped from the training mission, and confirm the spawn->geometry
binding (FUN_80078B14: instance[+0x0a] block index -> blkrec=0x8019F538+idx*44 ->
blkrec[+0x28] geometry ptr)."""
import struct, os
HERE = os.path.dirname(__file__)
BLK_BASE = 0x8019F538
INST_BASE = 0x8019FAB8
blk = open(os.path.join(HERE, "block_table_8019F538.bin"), "rb").read()
ins = open(os.path.join(HERE, "inst_table_8019FAB8.bin"), "rb").read()


def u16(b, o): return struct.unpack_from("<H", b, o)[0]
def s16(b, o): return struct.unpack_from("<h", b, o)[0]
def u32(b, o): return struct.unpack_from("<I", b, o)[0]


def nonzero(rec): return any(rec)

print("=== BLOCK RECORD TABLE @ 0x8019F538 (stride 44) ===")
blkrecs = {}
for i in range(len(blk) // 44):
    r = blk[i*44:(i+1)*44]
    if not nonzero(r):
        continue
    cnt0, cnt2, idx = u16(r, 0), u16(r, 2), u32(r, 4)
    gptr = u32(r, 0x28)
    blkrecs[i] = gptr
    print(f"  blk[{i:3d}] @0x{BLK_BASE+i*44:08x}: +0x00={cnt0:<5d} +0x02={cnt2:<5d}"
          f" +0x04(idx)={idx:<5d} +0x28(geom)=0x{gptr:08x}"
          + ("  <-- RAM ptr" if 0x80000000 <= gptr < 0x80200000 else ""))

print("\n=== INSTANCE / SPAWN TABLE @ 0x8019FAB8 (stride 44) ===")
print("  (hw0-2 ~pos, +0x0a=block index, +0x0e=type?; first u16==0xffff => empty)")
active = []
for i in range(len(ins) // 44):
    r = ins[i*44:(i+1)*44]
    if u16(r, 0) == 0xffff or not nonzero(r):
        continue
    hws = [u16(r, k*2) for k in range(8)]
    blkidx = u16(r, 0x0a)
    active.append((i, r))
    print(f"  ins[{i:3d}] @0x{INST_BASE+i*44:08x}: "
          f"x={s16(r,0):6d} y={s16(r,2):6d} z={s16(r,4):6d} "
          f"+06=0x{u16(r,6):04x} +08=0x{u16(r,8):04x} "
          f"+0a(blk)={blkidx:3d} +0c=0x{u16(r,0xc):04x} +0e=0x{u16(r,0xe):04x}")

print("\n=== BINDING CHECK (FUN_80078B14: ins[+0x0a] -> blk[idx][+0x28]) ===")
for i, r in active:
    bi = u16(r, 0x0a)
    gptr = blkrecs.get(bi, None)
    ok = gptr is not None and 0x80000000 <= gptr < 0x80200000
    print(f"  ins[{i:3d}] block_index={bi:3d} -> blk[{bi}] geom="
          + (f"0x{gptr:08x} {'VALID RAM ptr' if ok else 'not a ptr'}"
             if gptr is not None else "<no such block record>"))
