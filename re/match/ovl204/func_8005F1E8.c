#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    u32 *ot;                    /* +0x00 */
    s16 sx;                     /* +0x04 */
    s16 sy;                     /* +0x06 */
    s32 z;                      /* +0x08 */
} BbScratch;

extern DrawCtx D_801ADCCC;

/* billboard sprite at world point v: size (bit 0x800 = half height) scaled by depth */
void func_8005F1E8(SVECTOR *v, SprInfo *s, u32 *rgb, s32 size, s32 z)
{
    BbScratch *scr = (BbScratch *)0x1F800000;
    POLY_FT4 *p;
    s32 *otz;
    s32 w, h, n;

    scr->ot = D_801ADCCC.ot + z;
    gte_ldv0(v);
    gte_rtps();
    gte_stsxy(0x1F800004);
    otz = (s32 *)0x1F800008;
    gte_stszotz(otz);
    if (*otz == 0) {
        return;
    }
    if (size & 0x800) {
        w = ((size & 0x7FF) * 40) / *otz;
        h = w >> 1;
    } else {
        h = (size * 40) / *otz;
        w = h;
    }
    p = (POLY_FT4 *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(p + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    p->x0 = scr->sx - w;
    p->y0 = scr->sy - h;
    p->x1 = scr->sx + w;
    p->y1 = scr->sy - h;
    p->x2 = scr->sx - w;
    p->y2 = scr->sy + h;
    p->x3 = scr->sx + w;
    p->y3 = scr->sy + h;
    *(u32 *)&p->r0 = *rgb;
    p->clut = s->clut;
    p->tpage = s->tpage;
    p->u0 = s->u;
    p->v0 = s->v;
    p->u1 = s->u + s->w;
    p->v1 = s->v;
    p->u2 = s->u;
    p->v2 = s->v + s->h;
    p->u3 = s->u + s->w;
    p->v3 = s->v + s->h;
    setlen(p, 9);
    setcode(p, 0x2E);
    n = scr->z >> 1;
    if (n >= 0) {
        addPrim(scr->ot + n, p);
    }
}
