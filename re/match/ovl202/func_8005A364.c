#include "ac1/ac1.h"
/* ovl202 0x8005A364 - transform the n vertices of the current list (D_801A6544) into
 * D_801A6A48: screen xy, ot z, depth cue and a screen clip code (1 left, 2 right,
 * 4 above, 8 below of 320x240), with the GTE pipelined one vertex ahead. */

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    u8 clip;                    /* +0x06 */
    u8 pad07;
    s32 unk8;
    s32 dp;                     /* +0x0C */
} ClipVtx;

typedef struct {
    u8 pad00[0xC];
    union {
        s32 w;
        struct { u16 x, y; } h;
    } sxy;                      /* +0x0C */
    s32 dp;                     /* +0x10 */
    s32 otz;                    /* +0x14 */
} VtxSpad;

extern ClipVtx D_801A6A48[];
extern SVECTOR *D_801A6544;
extern void func_8005A330(SVECTOR *v, s32 *sxy, s32 *p, s32 *otz);

#define CLIP(o) \
    if ((s16)sp->sxy.h.x < 0) { \
        (o)->clip = 1; \
    } else if ((s16)sp->sxy.h.x > 320) { \
        (o)->clip = 2; \
    } else { \
        (o)->clip = 0; \
    } \
    if ((s16)sp->sxy.h.y < 0) { \
        (o)->clip |= 4; \
    } else if ((s16)sp->sxy.h.y > 240) { \
        (o)->clip |= 8; \
    }

void func_8005A364(s32 n)
{
    ClipVtx *o;
    SVECTOR *v;
    VtxSpad *sp;
    s32 *psxy;
    s32 *pdp;
    s32 *potz;

    sp = (VtxSpad *)0x1F800000;
    o = D_801A6A48;
    v = D_801A6544;
    func_8005A330(v, (s32 *)0x1F80000C, (s32 *)0x1F800010, (s32 *)0x1F800014);
    v++;
    n--;
    if (n != 0) {
        do {
        gte_ldv0(v);
        gte_rtps();
        o->dp = sp->dp;
        o->otz = sp->otz;
        o->sxy = sp->sxy.w;
        CLIP(o);
        v++;
        o++;
        psxy = (s32 *)0x1F80000C;
        pdp = (s32 *)0x1F800010;
        potz = (s32 *)0x1F800014;
        gte_stsxy(psxy);
        gte_stdp(pdp);
        gte_stszotz(potz);
        } while (--n != 0);
    }
    o->dp = sp->dp;
    o->otz = sp->otz;
    o->sxy = sp->sxy.w;
    CLIP(o);
}
