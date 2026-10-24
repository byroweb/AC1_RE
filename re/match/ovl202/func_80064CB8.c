#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    s32 sxy;
    s32 otz;
    s32 unk8;
    s32 dp;
} ScrVtx;

typedef struct {
    ScrVtx v[1729];
    s32 pad[3];
    s32 depth_cue; /* +0x6C1C = 0x801AD664 */
} ScrBuf;

extern ScrBuf D_801A6A48;
extern SVECTOR *D_801A6544;

void func_80064CB8(s32 n)
{
    ScrVtx *o;
    SVECTOR *v;

    o = D_801A6A48.v;
    v = D_801A6544;
    do {
        if (D_801A6A48.depth_cue) {
            gte_ldv0(v);
            gte_rtps();
            v++;
            gte_stsxy(&o->sxy);
            gte_stdp(&o->dp);
            gte_stszotz(&o->otz);
        } else {
            gte_ldv0(v);
            gte_rtps();
            v++;
            o->dp = 0;
            do {
                gte_stsxy(&o->sxy);
                gte_stszotz(&o->otz);
            } while (0);
        }
        o++;
    } while (--n);
}
