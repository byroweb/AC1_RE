#include "ac1/ac1.h"

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;                    /* +0x08 */
    s32 xy1;                    /* +0x0C */
} LineF2;

typedef struct {
    u32 tag;
    u32 code[2];
} DrMode;

typedef struct {
    u16 unk0;
    u16 tpage;                  /* +0x02 */
} LineTex;

typedef struct {
    u32 *ot;                    /* +0x00 */
    s16 depth;                  /* +0x04 */
    u8 unk6[4];
    u16 tpage;                  /* +0x0A */
    u8 unkC[4];
    u32 rgb;                    /* +0x10 */
    SVECTOR va;                 /* +0x14 */
    SVECTOR vb;                 /* +0x1C */
    u8 unk24[0x68 - 0x24];
    s32 sxy0;                   /* +0x68 */
    s32 sxy1;                   /* +0x6C */
    s32 z0;                     /* +0x70 */
    s32 z1;                     /* +0x74 */
    u8 unk78[4];
    s32 half;                   /* +0x7C */
} LineScratch;

#define SCR ((LineScratch *)0x1F800000)

extern DrawCtx D_801ADCCC;
extern void func_8005DEE4(SVECTOR *v);
extern void SetDrawMode(DrMode *p, s32 dfe, s32 dtd, s32 tpage, void *tw);

/* draw a 3D line a-b already projected into the scratchpad: near lines are split in
 * two halves and clipped by func_8005DEE4, far ones go out as one LINE_F2 */
void func_8005E278(SVECTOR *a, SVECTOR *b, LineTex *t, u32 *rgb)
{
    LineF2 *p;
    DrMode *dm;
    s32 n;

    if (SCR->z0 <= 160 || SCR->z1 <= 160) {
        SCR->depth = 2;
        SCR->rgb = *rgb;
        SCR->va = *a;
        SCR->vb.vx = (a->vx + b->vx) >> 1;
        SCR->vb.vy = (a->vy + b->vy) >> 1;
        SCR->vb.vz = (a->vz + b->vz) >> 1;
        SCR->tpage = t->tpage;
        SCR->half = 0;
        func_8005DEE4(&SCR->va);
        SCR->va = SCR->vb;
        SCR->vb = *b;
        SCR->half = 1;
        func_8005DEE4(&SCR->va);
        return;
    }
    p = (LineF2 *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(p + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    p->xy0 = SCR->sxy0;
    p->xy1 = SCR->sxy1;
    *(u32 *)&p->r0 = *rgb;
    setlen(p, 3);
    setcode(p, 0x42);
    n = (SCR->z0 + SCR->z1) >> 2;
    if (n >= 0) {
        addPrim(SCR->ot + n, p);
    }
    dm = (DrMode *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(dm + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    SetDrawMode(dm, 0, 1, t->tpage, 0);
    if (n >= 0) {
        addPrim(SCR->ot + n, dm);
    }
}
