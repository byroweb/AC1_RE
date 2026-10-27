/* include/ac1/frontend.h - overlay 201 (front end) pad block, and the equipped-parts
 * bytes the garage screens edit.  Opt-in: include after ac1/ac1.h.
 *
 * PadState: 35 matched sources declare 0x801A2550 locally (as PadState, PadRepeat,
 * PadSt or FePad, 0x20 or 0x28 bytes); 0x801A2554..0x801A2579 are also used as
 * scalars.  The poll func_8005B4B8 and the reset func_80050CE0 touch every field
 * below, so the block is 0x2C bytes.  uidialog.h's UiPad is an older, shorter view
 * of the same words (unk00 = latch, trig = press, held = prev).
 *
 * Use the struct where retail addresses the block from one base register (a field
 * touched twice in one block, or several fields together); the scalar D_801A25xx
 * form is what other functions compiled to.  Agents found both in the retail code.
 */
#ifndef AC1_FRONTEND_H
#define AC1_FRONTEND_H

#include "types.h"

typedef struct PadState {
    u32 latch;                  /* +0x00 buttons already reported: press = now & ~latch.  A set
                                 *       bit stays until released, or until the key-repeat
                                 *       countdown below runs out (pad 1 d-pad + bits 0-1, mask
                                 *       0xF003; pad 2 d-pad, mask 0xF0000000).  Matches call it
                                 *       used / latch */
    s16 repeat;                 /* +0x04 pad 1 repeat period, set to 6 / rep_div on a press */
    s16 first;                  /* +0x06 pad 1 first-repeat delay, 12 / rep_div, set only when the
                                 *       key was up last frame; counts down before repeat */
    s32 unk08;                  /* +0x08 cleared by func_80050CE0, otherwise unused so far */
    s16 repeat2;                /* +0x0C pad 2 repeat period (two-player setup, w2c09) */
    s16 first2;                 /* +0x0E pad 2 first-repeat delay */
    s16 quit;                   /* +0x10 non-zero ends the front-end frame loop (func_8009B414) */
    u16 pad12;
    u32 now;                    /* +0x14 raw buttons this frame, halves swapped when swap is set */
    u32 press;                  /* +0x18 0x801A2568 newly reported buttons (now & ~latch) */
    u32 prev;                   /* +0x1C 0x801A256C last frame's raw buttons.  Most matches name it
                                 *       held: `(press & k) && !(prev & k)` = a fresh press */
    u8 swap;                    /* +0x20 swap the two pads' halves (player 2 drives the menus);
                                 *       the poll reads it with lbu */
    u8 pad21[3];
    s32 rep_div;                /* +0x24 key-repeat divisor (6 / rep_div, 12 / rep_div; 0 = 1).
                                 *       Read in ~60 sources as the scalar D_801A2574 */
    s8 unk28;                   /* +0x28 cleared by func_80050CE0 */
    s8 unk29;                   /* +0x29 cleared by func_80050CE0 */
    u8 pad2a[2];
} PadState;
AC1_ASSERT_SIZE(PadState, 0x2C);

/* 0x80031A58 g_ac_parts: u8[12], the part id equipped in each slot, 255 = empty.
 * Slots named where two or more matches agree (part tables per notes from the
 * assembly, shop and AC-test agents):
 *   0 core (core table 0x800B944C), 3 arms (ids >= 8 are weapon arms), 2 head,
 *   8 generator (0x800BAFF4), 11 legs (0x800B94F4, max load).
 * One source each: 1 booster, 9 FCS, 6/7 the weapon slots the preview model reads. */
#define AC_PART_CORE        0
#define AC_PART_HEAD        2
#define AC_PART_ARMS        3
#define AC_PART_GENERATOR   8
#define AC_PART_LEGS        11
#define AC_PART_SLOTS       12
#define AC_PART_NONE        255

/* Part-spec tables (overlay 201 data, g_parts_*): one per part category, each row
 * starting with this 0x28-byte header, then category-specific stats (weight u16 at
 * +0x28 in head/core/legs/arms).  Layout from the wave-13 match of func_80075898,
 * checked against the retail rows (e.g. core row 0: "XCA-00", "CORE UNIT", 61500).
 *   category  table       stride  rows are
 *    0        0x800B944C  56      core           5  0x800BABAC  48  booster
 *    1        0x800B94F4  72      legs           6  0x800BA3F4  52  back weapons
 *    2        0x800B9DF4  52      arms           7  0x800BA134  44  arm weapons (row 0 "NO WEAPON")
 *    3        0x800BAADC  52      laser blades   8  0x800BACCC  96  FCS
 *    4        0x800B91F4  60      head          10  0x800BAFCC  48  generators
 * Strides differ, so the tables are typed u8[] here; index them with the stride.
 * The category numbers are the parts list's, not the AC_PART_* slots above.
 * Also read with them: 24-byte weapon stat records at 0x800BB300 (and through
 * 0x800BB3C0, 0x800BB540), string-pointer tables 0x800BBA34..0x800BBAD8. */
typedef struct PartHdr {
    char name[18];              /* +0x00 part id, e.g. "HD-01-SRVT" */
    char kind[18];              /* +0x12 category text, e.g. "HEAD UNIT" */
    s32 price;                  /* +0x24 */
} PartHdr;
AC1_ASSERT_SIZE(PartHdr, 0x28);

#define PART_CAT_CORE       0
#define PART_CAT_LEGS       1
#define PART_CAT_ARMS       2
#define PART_CAT_BLADE      3
#define PART_CAT_HEAD       4
#define PART_CAT_BOOSTER    5
#define PART_CAT_BACK       6
#define PART_CAT_ARM_WEAPON 7
#define PART_CAT_FCS        8
#define PART_CAT_GENERATOR  10

/* --- symbols: GENERATED by tools/match/ctx.py gen from symbols.tsv; do not edit --- */
extern PadState g_fe_pad;                                /* 0x801A2550 */
/* --- end of generated symbols --- */

#endif
