#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    s32 sxy;                    /* +0x00 */
    s32 otz;                    /* +0x04 */
    s32 unk8;
    s32 dp;                     /* +0x0C depth-cue factor */
} ScrVtx;

extern ScrVtx D_801A6A48[];

/* func_80062FFC plus the depth-cue factor */
void func_80063B94(SVECTOR *v, s32 n)
{
    ScrVtx *o;

    o = D_801A6A48;
    do {
        gte_ldv0(v);
        gte_rtps();
        v++;
        gte_stsxy(&o->sxy);
        gte_stdp(&o->dp);
        gte_stszotz(&o->otz);
        o++;
    } while (--n);
}
