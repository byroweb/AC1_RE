#!/usr/bin/env python3
"""
title_build.py — bake the Farsi hub category titles into the [RTL] disc.

Approach (zero new game code): the USA build never renders Japanese kanji, but it
still loads the kanji font sheet (MENU_TIM.T entry 4) to VRAM (576,256). We
overwrite the top 72 rows of that dead sheet with six crisp 128x24 Farsi word
cells, and repoint the hub title descriptor table (FDAT entry-201 @0x800B6E4C) to
read from there (tpage 0x39 = VRAM (576,256)). No overlay code, no upload hook.

Operates ON TOP of the existing [RTL] disc (preserves the name-shaper FDAT patch):
  1. MENU_TIM.T entry 4: write the 256x72 4bpp word band @img 0x7F220; fix the
     entry-4 trailing checksum @0x9F7FC; inject entry-4 sectors into [RTL].bin.
  2. FDAT.T entry-201: write the 6 retargeted descriptors @0x800B6E4C; fix the
     entry-201 checksum; inject entry-201 sectors into [RTL].bin.
"""
import os, struct, shutil, sys

HERE   = os.path.dirname(os.path.abspath(__file__))
BIN    = "/home/byron/Desktop/Armored_Core_Hacks/AC_1_USA_test/Armored Core (v1.1) [RTL].bin"
MENU_SRC = "/tmp/MENU_TIM_original.T"          # pristine MENU_TIM (== retail, verified)
FDAT_SRC = os.path.join(HERE, "fdat_patched.T")# current FDAT (has the name shaper)
BAND   = "/tmp/title_band.raw"                 # 256x72 4bpp word band (9216B)
DESC   = "/tmp/desc_kanji.bin"                 # 6x12B descriptors, tpage 0x39

SECTOR=2352; DOFF=24; DSIZE=2048; SEED=0x12345678

# --- MENU_TIM geometry ---
MENU_FIRST_BINSEC = 100295                     # MENU_TIM.T first sector in the BIN
E4_IMG   = 0x7F220                             # entry-4 image data file offset
E4_TRAIL = 0x9F7FC                             # entry-4 trailing checksum file offset
E4_BASE  = 254*2048                            # entry-4 start (sector 254)
E4_SEC0, E4_SEC1 = 254, 318                    # entry-4 file sectors (inclusive)

# --- FDAT entry-201 geometry (from build_rtl_patch.py / REFERENCE) ---
FDAT_FIRST_BINSEC = 72189
E201_FLAT  = 25_548_800
E201_TRAIL = 26_091_516                        # entry-201 trailing checksum flat off
DESC_FLAT  = 25_991_340                        # 0x800B6E4C -> flat
E201_SEC0, E201_SEC1 = 12475, 12739            # entry-201 file sectors (inclusive)

def fix_checksum(buf, base, trail):
    s = SEED
    for off in range(base, trail, 4):
        s = (s + struct.unpack_from("<I", buf, off)[0]) & 0xFFFFFFFF
    struct.pack_into("<I", buf, trail, s)
    return s

def inject(binf, filebuf, fsec0, fsec1, binsec0):
    """Write file sectors [fsec0..fsec1] of filebuf into the BIN at binsec0+offset."""
    with open(binf, "r+b") as f:
        for s in range(fsec0, fsec1+1):
            data = filebuf[s*DSIZE:(s+1)*DSIZE]
            assert len(data)==DSIZE
            f.seek((binsec0 + (s - fsec0)) * SECTOR + DOFF)
            f.write(data)

def main():
    for p in (BIN, MENU_SRC, FDAT_SRC, BAND, DESC):
        if not os.path.exists(p): sys.exit("missing: "+p)
    # backup the working disc once
    bak = BIN + ".pre-title.bak"
    if not os.path.exists(bak): shutil.copyfile(BIN, bak); print("backup ->", bak)

    band = open(BAND, "rb").read();  assert len(band)==9216
    desc = open(DESC, "rb").read();  assert len(desc)==72

    # 1) MENU_TIM entry 4
    menu = bytearray(open(MENU_SRC, "rb").read())
    menu[E4_IMG:E4_IMG+len(band)] = band
    ck = fix_checksum(menu, E4_BASE, E4_TRAIL)
    inject(BIN, menu, E4_SEC0, E4_SEC1, MENU_FIRST_BINSEC + E4_SEC0)
    print(f"MENU_TIM entry-4: band written, checksum {ck:#010x}, injected sectors {E4_SEC0}-{E4_SEC1}")

    # 2) FDAT entry-201 descriptors
    fdat = bytearray(open(FDAT_SRC, "rb").read())
    fdat[DESC_FLAT:DESC_FLAT+len(desc)] = desc
    ck2 = fix_checksum(fdat, E201_FLAT, E201_TRAIL)
    inject(BIN, fdat, E201_SEC0, E201_SEC1, FDAT_FIRST_BINSEC + E201_SEC0)
    print(f"FDAT entry-201: descriptors written, checksum {ck2:#010x}, injected sectors {E201_SEC0}-{E201_SEC1}")
    print("done ->", BIN)

if __name__ == "__main__":
    main()
