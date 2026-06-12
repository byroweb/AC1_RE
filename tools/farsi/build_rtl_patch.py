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
ROOT   = os.path.dirname(os.path.dirname(HERE))
SRC_T  = os.path.join(ROOT, "fdat_extracted.T")
PAT_T  = os.path.join(ROOT, "fdat_patched.T")
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

# --- Farsi name shaper integration (compiled from C at build time) ----------
# Placement (all in VERIFIED-free overlay padding; see docs/AC1_NAME_SHAPER.md):
#   shape_name code   -> 0x800BC740 (436B padding block)
#   shape_name tables -> 0x80081F70 (200B padding block)
#   name_input_handler-> 0x80082190 (432B; ends exactly at the row-0 keyboard
#                        glyph string @0x80082340 -- DO NOT exceed)
#   ordinal SELTAB    -> 0x8004C700 (replaces the old glyph-byte selection table)
# The trampoline @0x80082168 (jal 0x80082190; j 0x800823F4) is already in the .T.
SHAPE_CODE   = 0x800BC740
SHAPE_TABLES = 0x80081F70
HANDLER      = 0x80082190
SELTAB_ADDR  = 0x8004C700
SHAPE_NAME_ENTRY = 0x800BC740      # nm: shape_name lands here (16-aligned, no pad)
DRAW_CONFIRM = 0x8005D8D4
NAME_XPOS    = 0x80081FD0           # right-justify routine, in the proven 200B
                                    # padding block right after the shaper tables
                                    # (the 261B block @0x800BEB53 is a boot table -
                                    # overwriting it hangs boot; do NOT use it)
CURSOR_UPDATE = 0x80083A28          # repointed to `j NAME_XPOS`

# Ordinal selection table: 4 rows x 17 cols. 0..32 letter ordinals
# (farsi_runtime_shape.KEYBOARD order), 33 space, 34..43 Persian digits ۰..۹,
# 0xFC END, 0xFF blank. Derived by translating the live glyph SELTAB letter->ordinal.
SELTAB = bytes.fromhex(
    "100f0e0d0c0b0a09080706050403020100"   # row0: ص..ا  (col0..16)
    "ff201f1e1d1c1b1a191817161514131211"   # row1: blank, آ..ض
    "ffffffffffffff22232425262728292a2b"   # row2: digits ۰..۹ (col7..16)
    "2121fcfcffffffffffffffffffffffffff")  # row3: space,space,END,END,blank...

def _compile_shaper():
    """Compile farsi_name_shape.c (split code/tables) + farsi_name_input.c into
    flat overlay binaries; return {addr: bytes}. Raises on toolchain error."""
    import subprocess, tempfile
    GCC = "mipsel-linux-gnu-gcc"; OBJCOPY = "mipsel-linux-gnu-objcopy"
    CFLAGS = ("-march=r3000 -mips1 -mfp32 -EL -G0 -Os -falign-functions=4 "
              "-ffreestanding -fno-builtin -fno-pic -mno-abicalls -nostdlib").split()
    d = tempfile.mkdtemp()
    # shaper: code @SHAPE_CODE, const tables @SHAPE_TABLES (absolute refs resolve)
    lds = os.path.join(d, "shaper.ld")
    with open(lds, "w") as f:
        f.write(f"ENTRY(shape_name)\nSECTIONS {{\n"
                f"  .text 0x{SHAPE_CODE:08X} : {{ *(.text) *(.text.*) }}\n"
                f"  .rodata 0x{SHAPE_TABLES:08X} : {{ *(.rodata) *(.rodata.*) "
                f"*(.data) *(.data.*) *(.sdata*) *(.srodata*) }}\n"
                f"  /DISCARD/ : {{ *(.reginfo) *(.MIPS.abiflags) *(.pdr) "
                f"*(.comment) *(.note.*) *(.gnu*) }}\n}}\n")
    elf = os.path.join(d, "shaper.elf")
    subprocess.run([GCC, *CFLAGS, "-T", lds, "-o", elf,
                    os.path.join(HERE, "farsi_name_shape.c")], check=True)
    code = os.path.join(d, "code.bin"); tab = os.path.join(d, "tab.bin")
    subprocess.run([OBJCOPY, "-O", "binary", "-j", ".text", elf, code], check=True)
    subprocess.run([OBJCOPY, "-O", "binary", "-j", ".rodata", elf, tab], check=True)
    # handler @HANDLER, bound to shape_name + draw_confirm absolute addresses
    hlds = os.path.join(d, "h.ld")
    with open(hlds, "w") as f:
        f.write(f"ENTRY(name_input_handler)\nSECTIONS {{\n"
                f"  .text 0x{HANDLER:08X} : {{ *(.text) *(.text.*) *(.rodata) "
                f"*(.rodata.*) *(.data) *(.data.*) }}\n"
                f"  /DISCARD/ : {{ *(.reginfo) *(.MIPS.abiflags) *(.pdr) "
                f"*(.comment) *(.note.*) *(.gnu*) }}\n}}\n")
    helf = os.path.join(d, "h.elf")
    subprocess.run([GCC, *CFLAGS, "-T", hlds,
                    f"-Wl,--defsym=shape_name=0x{SHAPE_NAME_ENTRY:08X}",
                    f"-Wl,--defsym=draw_confirm=0x{DRAW_CONFIRM:08X}",
                    "-o", helf, os.path.join(HERE, "farsi_name_input.c")], check=True)
    hbin = os.path.join(d, "h.bin")
    subprocess.run([OBJCOPY, "-O", "binary", "-j", ".text", helf, hbin], check=True)
    # name_xpos: proportional right-justify (sums fmet adv). HAND-ASSEMBLED (GCC
    # -Os emitted ~192B; the asm is 96B to fit the 200B block tail). Tail-jumps to
    # cursext for the label fix.
    AS = "mipsel-linux-gnu-as"; LD = "mipsel-linux-gnu-ld"
    xlds = os.path.join(d, "x.ld")
    with open(xlds, "w") as f:
        f.write(f"ENTRY(name_xpos)\nSECTIONS {{ .text 0x{NAME_XPOS:08X} : "
                f"{{ *(.text) }} /DISCARD/ : {{ *(.reginfo) *(.pdr) "
                f"*(.MIPS.abiflags) *(.comment) }} }}\n")
    xo = os.path.join(d, "x.o"); xelf = os.path.join(d, "x.elf")
    xbin = os.path.join(d, "x.bin")
    subprocess.run([AS, "-march=r3000", "-mips1", "-EL", "-o", xo,
                    os.path.join(HERE, "farsi_name_xpos.s")], check=True)
    subprocess.run([LD, "-EL", "-T", xlds, "-o", xelf, xo], check=True)
    subprocess.run([OBJCOPY, "-O", "binary", "-j", ".text", xelf, xbin], check=True)
    code_b = open(code, "rb").read(); tab_b = open(tab, "rb").read()
    hbin_b = open(hbin, "rb").read(); xbin_b = open(xbin, "rb").read()
    # size guards: handler must not reach the row-0 glyph string at 0x80082340
    assert HANDLER + len(hbin_b) <= 0x80082340, f"handler too big: {len(hbin_b)}B"
    assert SHAPE_CODE + len(code_b) <= 0x800BC8EA, f"shape code too big: {len(code_b)}B"
    assert SHAPE_TABLES + len(tab_b) <= 0x80082038, f"shape tables too big: {len(tab_b)}B"
    assert NAME_XPOS + len(xbin_b) <= 0x80082038, f"name_xpos too big: {len(xbin_b)}B"
    assert SHAPE_TABLES + len(tab_b) <= NAME_XPOS, "shaper tables collide with name_xpos"
    print(f"  compiled shaper code {len(code_b)}B, tables {len(tab_b)}B, "
          f"handler {len(hbin_b)}B, name_xpos {len(xbin_b)}B")
    # repoint cursor.update -> `j NAME_XPOS; nop` (+ nop pad to original 48B span)
    j = 0x08000000 | ((NAME_XPOS >> 2) & 0x03FFFFFF)
    cur_upd = struct.pack("<I", j) + b"\x00" * 44
    return {SHAPE_CODE: code_b, SHAPE_TABLES: tab_b, HANDLER: hbin_b,
            SELTAB_ADDR: SELTAB, NAME_XPOS: xbin_b, CURSOR_UPDATE: cur_upd}

# --- the patches (runtime addr -> bytes). Single source of truth. ---------
PATCHES = {
    # cursor.update() rewrite: right-justify name + reversed cursor (see farsi_name_rtl.c).
    # ALSO tail-jumps to cursext (0x80082130) to set the label type=6 every frame —
    # the proven post-build label fix (live poke of 0x801A73F2=6). This method only
    # runs on the name screen, so it is the perfect name-screen-specific hook.
    # Last word is `j 0x80082130` (was `jr ra`); delay slot `sh v0,0x7500` unchanged.
    0x80083A28: bytes.fromhex(
        "7c00828c00000000960045901a80013c002905007800022423104500"
        "607322a488000224231045004c080208007522a4"),
    # cursext: at==0x801A0000 still held from cursor.update; set label type=6 AND
    # right-justify the label (X=72 puts نام خلبان's right edge inside the box), return.
    0x80082130: bytes.fromhex(
        "06000224"   # addiu v0,zero,6
        "f27322a4"   # sh   v0,0x73F2(at)    ; *(0x801A73F2) label type = 6
        "48000224"   # addiu v0,zero,72
        "307422a4"   # sh   v0,0x7430(at)    ; *(0x801A7430) label X = 72 (right-justify)
        "0800e003"   # jr   ra
        "00000000"), # nop
    # NOTE: box X via caller 0x8005C284 'lh a0,0xe(s2)' is REJECTED — 0x8005C2A8 is
    # a GENERIC W=144 box builder (boot test: the memory-card SLOT boxes moved too).
    # Clean fix needs the PILOT box's descriptor X source (per-element), TODO task #6.

    # label type 7->6 @0x8005D868 REJECTED — boot test: shared type-7 constructor,
    # broke the memory-card SLOT labels (ran past their \0). Needs a name-screen-
    # SPECIFIC type flip on the label element (0x801A73E8) post-construction. TODO.
    # (string patch also held back until the type flip is label-specific)
    # keyboard SPC/END row: strptr table @0x800B8608, entry[3]=0x800823D4 ("SPC END").
    # relocate "فاصله تمام" into free overlay space 0x800820E8 and repoint the entry.
    # 5 spaces between the words so SPC(فاصله) and END(تمام) read as two distinct keys.
    0x800820E8: bytes.fromhex("eddeb581cd2020202020e081e28b3e"),  # فاصله _____ تمام >
    0x800B8614: bytes.fromhex("e8200880"),                  # ptr -> 0x800820E8

    # NOTE: a blanket per-glyph keyboard shift (byte2 of each row's spacer glyphs)
    # was tried 2026-06-07 and REVERTED — it pushed the whole alphabet too far left
    # and threw glyphs like ا (alef) out of their cells. The original spacing only
    # needs minor per-glyph tweaks, applied surgically (not a blanket offset).
    # Each row is a 1-byte glyph string: visible letter (>=0x80) + two variable-width
    # blank spacer glyphs (byte1=Y, byte2=X advance). To nudge ONE letter right,
    # shrink its byte2 spacer; left, grow it. Rows: 0x80082340 (top), 0x80082374.

    # --- PILOT NAME box -> right + label -> نام خلبان (name-screen-SPECIFIC hooks) -
    # Both the box and the label are built by SHARED constructors (patching them
    # directly broke the memory-card screens). Instead, install two trampolines in
    # free overlay space that act ONLY on the PILOT NAME element, keyed on a unique
    # argument, then fall through unchanged for every other caller.
    #
    # BOX: caller @0x8005C290 loads X/Y/W/H from descriptor s2 then jal box_init.
    #   Replace `lh a3,0x14(s2)` with `j boxhook`; boxhook redoes the load, and if
    #   a1(Y)==178 (only the PILOT NAME box) forces a0(X)=168 (right-edge align to
    #   the keyboard panel: 24+288-144). Other boxes (keyboard Y=40, card slots) pass.
    0x80082108: bytes.fromhex(
        "14004786"   # lh   a3,0x14(s2)      ; displaced
        "b2000124"   # addiu at,zero,178     ; Y of PILOT NAME box
        "0200a114"   # bne  a1,at,+2         ; if Y!=178 skip override
        "00000000"   # nop
        "a8000424"   # addiu a0,zero,168     ; X = 312-144
        "a6700108"   # j    0x8005C298       ; return
        "00000000"), # nop
    0x8005C290: bytes.fromhex("42080208"),  # j 0x80082108 (boxhook)

    # alef nudges (user review): ا and آ sat too far left. Both are the LAST glyph
    # in their row, so shrinking their X-spacer nudges them right with no cascade.
    0x80082372: bytes.fromhex("38"),  # ا (row0 last) X-spacer 0x3d->0x38 (~+2px)
    0x800823A5: bytes.fromhex("2c"),  # آ (row1 last) X-spacer 0x31->0x2c (~+2px)

    # (label type flip is done by cursext above, tail-called from cursor.update)
    # shaped "نام خلبان" + 0x3e terminator into the (runtime type-6) label slot
    0x8004C6F4: bytes.fromhex("e48184de9f20e081e53e"),

    # name-entry default cursor: the init @0x80081144 sets col=16 (0x9a(s3)) and
    # @0x8008114C `addiu v0,zero,7` -> row=7 (0x9b(s3)=0x801A28F3), parking the
    # cursor off-grid. Change the row immediate 7->0 so it starts on ALEF (col16,
    # row0 = SELTAB[16]). 1-byte patch on the addiu immediate (LE low byte).
    0x8008114C: bytes.fromhex("00"),   # addiu v0,zero,7 -> addiu v0,zero,0
}
# expected ORIGINAL bytes (safety check before patching)
EXPECT = {
    0x80083A28: bytes.fromhex(
        "7c00828c000000009600459008000224ff00a330030062140011050007"
        "00052400110500080042240800e003480082ac"),
    0x8005D868: bytes.fromhex("07000324"),     # addiu v1, zero, 7
    0x8005D86C: bytes.fromhex("0a0043a4"),     # sh v1, 0xa(v0)
    0x8005C290: bytes.fromhex("14004786"),     # lh a3, 0x14(s2)
    0x80082108: bytes.fromhex("00"*28),        # free (boxhook target)
    0x80082372: bytes.fromhex("3d"),           # ا X-spacer (stock)
    0x800823A5: bytes.fromhex("31"),           # آ X-spacer (stock)
    0x80082130: bytes.fromhex("00"*24),        # free (cursext target)
    0x8004C6F4: bytes.fromhex("50494c4f54204e414d45"),   # "PILOT NAME"
    0x8008114C: bytes.fromhex("07"),   # addiu v0,zero,7 (cursor default row)
    0x800820E8: bytes.fromhex("000000000000000000000000000000"),  # free (15 zeros)
    0x800B8614: bytes.fromhex("d4230880"),               # -> 0x800823D4
    0x80082340: bytes.fromhex(  # row0 original (ص ش س ژ ز ر ذ د خ ح چ ج ث ت پ ب ا)
        "b47b31b07d30ac7d30aa7d3ba87d3ba67d3ba47d3aa27d3a9e7d389a7d38"
        "967d38927d388e7d348a7d34867d34827d34807d3d3e"),
    0x80082374: bytes.fromhex(  # row1 original (ض ط ظ ع غ ف ق ک گ ل م ن و ه ی آ)
        "2020f27d3dee7d36ea7d3ae87d3ae47d37e07d39dc7d37d87d35d47d35"
        "d07d36cc7d34c87d39c47d39c07d36bc7d36b87b313e"),
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
    print("0) compile + merge Farsi name shaper")
    _shaper = _compile_shaper()
    PATCHES.update(_shaper)
    # shape code/tables land in verified-zero padding -> EXPECT zeros. The handler
    # (0x80082190) and SELTAB (0x8004C700) overwrite the prior name-handler bake
    # already in fdat_extracted.T, so they are intentionally NOT EXPECT-checked.
    EXPECT[SHAPE_CODE]   = b"\x00" * len(_shaper[SHAPE_CODE])
    EXPECT[SHAPE_TABLES] = b"\x00" * len(_shaper[SHAPE_TABLES])
    EXPECT[NAME_XPOS]    = b"\x00" * len(_shaper[NAME_XPOS])
    # CURSOR_UPDATE (0x80083A28) overwrites the prior RTL cursor.update with a jump
    # to name_xpos; its EXPECT is the stock original (already in EXPECT dict).
    print("1) patch .T"); patch_T()
    print("2) reinsert into BIN copy"); reinsert()
    print(f"DONE -> {CUE_OUT}")
