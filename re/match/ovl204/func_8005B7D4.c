/* ovl204 0x8005B7D4  size 224  zero-agent sweep (tools/loop/sweep.py): neighbour+imm draft of ovl202 0x8005C630 (ovl202/func_8005C630.c, similarity 0.964), 2026-10-21 */
#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")

typedef struct {
    u8 unk0[0x40];
    VECTOR lv;                  /* +0x40 */
    u16 sx;                     /* +0x50 */
    u16 sy;                     /* +0x52 */
    s32 otz;                    /* +0x54 */
} ClipScratch;

#define SCR ((ClipScratch *)0x1F800000)

/* screen-clip code of v: 1/2 left/right of 0..319, 4/8 above/below 0..239,
 * 16 = behind the eye; -1 when far behind */
s32 func_8005B7D4(SVECTOR *v)
{
    ClipScratch *scr;
    s32 *otz;
    s32 fl;

    scr = SCR;
    gte_ldv0(v);
    gte_rtps();
    gte_stsxy(0x1F800050);
    otz = (s32 *)0x1F800054;
    gte_stszotz(otz);
    fl = 0;
    if (*otz == 0) {
        gte_stlvnl(0x1F800040);
        if (SCR->lv.vz < -10240) {
            return -1;
        }
        fl = 16;
    }
    if ((s16)scr->sx < 0) {
        fl |= 1;
    } else if ((s16)scr->sx >= 158) {
        fl |= 2;
    }
    if ((s16)scr->sy < 0) {
        fl |= 4;
    } else if ((s16)scr->sy >= 120) {
        fl |= 8;
    }
    return fl;
}
