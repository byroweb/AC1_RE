#!/usr/bin/env python3
"""
build_rtl_patch.py — bake overlay patches into a bootable AC1 disc copy.

Pipeline:
  1. copy clean fdat_extracted.T -> fdat_patched.T
  2. apply byte patches (by RUNTIME address) into entry-201 region of the .T
  3. recompute entry-201 trailing checksum (seed 0x12345678 + word sum)
  4. copy the original BIN -> patched BIN, reinsert the FDAT sectors
     (Mode2/Form1: 2048 data bytes at offset 24 of each 2352-byte sector)
  5. write a .cue for the patched BIN

EDC/ECC of changed sectors are left stale (DuckStation tolerates). Re-run any
time; PATCHES is the single source of truth.  Addresses verified live; base
0x8004ADA0 from patch_draw_string.py (draw_string 0x8006599c @ entry off 109564).
"""
import os, shutil, struct, sys

HERE   = os.path.dirname(os.path.abspath(__file__))
SRC_T  = os.path.join(HERE, "fdat_extracted.T")
PAT_T  = os.path.join(HERE, "fdat_patched.T")
BIN_IN = "/home/byron/Desktop/Armored_Core_Hacks/AC_1_USA_test/Armored Core (v1.1).bin"
BIN_OUT= "/home/byron/Desktop/Armored_Core_Hacks/AC_1_USA_test/Armored Core (v1.1) [RTL].bin"
CUE_OUT= "/home/byron/Desktop/Armored_Core_Hacks/AC_1_USA_test/Armored Core (v1.1) [RTL].cue"

# --- FDAT / entry-201 geometry ---
BASE        = 0x8004ADA0          # runtime base of overlay entry 201
E201_FLAT   = 25_548_800          # entry-201 start in the flat .T
E201_SIZE   = 542_720
SEED        = 0x12345678
FDAT_FIRST  = 72189               # first FDAT sector in the BIN
SECTOR_RAW  = 2352
DATA_OFFSET = 24
DATA_SIZE   = 2048

def flat(rt): return E201_FLAT + (rt - BASE)

# --- the patches (runtime addr -> bytes). Single source of truth. ---------
PATCHES = {
    # cursor.update() rewrite: right-justify name + reversed cursor (see farsi_name_rtl.c)
    0x80083A28: bytes.fromhex(
        "7c00828c00000000960045901a80013c002905007800022423104500"
        "607322a488000224231045000800e003007522a4"),
}
# expected ORIGINAL bytes (safety check before patching)
EXPECT = {
    0x80083A28: bytes.fromhex(
        "7c00828c000000009600459008000224ff00a330030062140011050007"
        "00052400110500080042240800e003480082ac"),
}

def patch_T():
    shutil.copyfile(SRC_T, PAT_T)
    with open(PAT_T, "r+b") as f:
        for rt, new in PATCHES.items():
            off = flat(rt)
            f.seek(off); cur = f.read(len(new))
            if rt in EXPECT and cur != EXPECT[rt]:
                sys.exit(f"ABORT: 0x{rt:08X} flat {off} != expected original "
                         f"({cur.hex()})")
            f.seek(off); f.write(new)
            print(f"  patched 0x{rt:08X} (flat {off:,}) {len(new)}B")
        # checksum
        nwords = E201_SIZE // 4
        f.seek(E201_FLAT)
        words = struct.unpack(f"<{nwords}I", f.read(E201_SIZE))
        s = SEED
        for w in words[:-1]:
            s = (s + w) & 0xFFFFFFFF
        f.seek(E201_FLAT + E201_SIZE - 4); f.write(struct.pack("<I", s))
        print(f"  entry-201 checksum -> 0x{s:08X}")

def reinsert():
    shutil.copyfile(BIN_IN, BIN_OUT)
    t = open(PAT_T, "rb").read()
    nsec = len(t) // DATA_SIZE
    with open(BIN_OUT, "r+b") as b:
        for i in range(nsec):
            b.seek((FDAT_FIRST + i) * SECTOR_RAW + DATA_OFFSET)
            b.write(t[i*DATA_SIZE:(i+1)*DATA_SIZE])
    print(f"  reinserted {nsec} FDAT sectors into patched BIN")
    with open(CUE_OUT, "w") as c:
        c.write(f'FILE "{os.path.basename(BIN_OUT)}" BINARY\n'
                f'  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')

if __name__ == "__main__":
    print("1) patch .T"); patch_T()
    print("2) reinsert into BIN copy"); reinsert()
    print(f"DONE -> {CUE_OUT}")
