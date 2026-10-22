#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")


typedef struct RibNode {
    SVECTOR a;                  /* +0x00 */
    SVECTOR b;                  /* +0x08 */
    u8 va;                      /* +0x10 texture v at a */
    u8 vb;                      /* +0x11 texture v at b */
    u8 pad12[2];
    s32 sxy;                    /* +0x14 saved screen xy of b */
    s32 z;                      /* +0x18 saved otz of b */
} RibNode;                      /* 0x1C, the next level follows directly */

typedef struct {
    u32 *ot;                    /* +0x00 */
    u16 depth;                  /* +0x04 */
    s16 width;                  /* +0x06 */
    u16 clut;                   /* +0x08 */
    u16 tpage;                  /* +0x0A */
    u8 u0;                      /* +0x0C */
    u8 u1;                      /* +0x0D */
    u8 unkE[2];
    u32 rgb;                    /* +0x10 */
    RibNode node;               /* +0x14 first level */
    u8 unk30[0x68 - 0x30];
    union {
        s32 xy;
        struct {
            s16 x, y;
        } v;
    } s0;                       /* +0x68 */
    union {
        s32 xy;
        struct {
            s16 x, y;
        } v;
    } s1;                       /* +0x6C */
    s32 z0;                     /* +0x70 */
    s32 z1;                     /* +0x74 */
    POLY_FT4 *prev;             /* +0x78 last quad of the strip */
    s32 half;                   /* +0x7C */
} RibScratch;

#define SCR ((RibScratch *)0x1F800000)
#define SH1(y, x) if ((x) > 0) { y = (x) - 1; } else if ((x) < 0) { y = (x) + 1; } else { y = (x); }
#define SH2(y, x) if ((x) <= 0) { if ((x) < 0) { y = (x) + 1; } else { y = (x); } } else { y = (x) - 1; }
#define SH3(y, x) if ((x) <= 0) { if ((x) < 0) { y = (x) + 1; } else { y = (x); } } else { y = (x) - 1; }
#define SH4(y, x) if ((x) <= 0) { if ((x) < 0) { y = (x) + 1; } else { y = (x); } } else { y = (x) - 1; }

extern DrawCtx D_801ADCCC;
extern s32 SquareRoot0(s32 a);
extern void func_8005E278(SVECTOR *a, SVECTOR *b, SprInfo *t, u32 *rgb);
extern void func_8005E588(RibNode *nd);
extern MATRIX D_801B4F9C;

#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define BIGGER(x, y) (ABS(x) > ABS(y))
#define SGN(x) ((x) >= 0 ? 1 : -1)

/* textured ribbon from a to b, screen width `width` (0: plain line); near ones are
 * split and clipped by func_8005E588, far ones go out as one POLY_FT4 */
void func_8005EB3C(SVECTOR *a, SVECTOR *b, SprInfo *t, u32 *rgb, s32 width)
{
    RibScratch *scr = SCR;
    POLY_FT4 *p;
    s32 n;
    s32 dx, dy, len;
    s32 a0, b0, c0, d0, a1, b1, c1, d1;

    if (D_801B4F9C.t[1] >= (a->vy + b->vy) >> 1) {
        scr->ot = D_801ADCCC.ot + 120;
    } else {
        scr->ot = D_801ADCCC.ot + 100;
    }
    gte_ldv0(a);
    gte_rtps();
    gte_stsxy(&scr->s0);
    gte_stszotz(&scr->z0);
    gte_ldv0(b);
    gte_rtps();
    gte_stsxy(&scr->s1);
    gte_stszotz(&scr->z1);
    if (scr->z0 == 0 && scr->z1 == 0) {
        return;
    }
    if (width == 0) {
        func_8005E278(a, b, t, rgb);
    }
    if (scr->z0 <= 160 || scr->z1 <= 160) {
        scr->prev = 0;
        scr->depth = 2;
        scr->width = width;
        scr->clut = t->clut;
        scr->tpage = t->tpage;
        scr->u0 = t->u;
        scr->u1 = t->u + t->w;
        scr->node.va = t->v;
        scr->node.vb = t->v + (t->h >> 1);
        scr->rgb = *rgb;
        scr->node.a = *a;
        scr->node.b.vx = (a->vx + b->vx) >> 1;
        scr->node.b.vy = (a->vy + b->vy) >> 1;
        scr->node.b.vz = (a->vz + b->vz) >> 1;
        scr->half = 0;
        func_8005E588(&scr->node);
        scr->node.va = scr->node.vb;
        scr->node.vb = t->v + t->h;
        scr->node.a = scr->node.b;
        scr->node.b = *b;
        scr->half = 1;
        func_8005E588(&scr->node);
        return;
    }
    dx = scr->s1.v.x - scr->s0.v.x;
    dy = scr->s1.v.y - scr->s0.v.y;
    len = SquareRoot0(dx * dx + dy * dy);
    if (len == 0) {
        len = 1;
    }
    a0 = (dx * ((width * 40) / scr->z0)) / len;
    b0 = (dy * ((width * 40) / scr->z0)) / len;
    if (a0 == 0 && b0 == 0) {
        if (BIGGER(dx, dy)) {
            a0 = SGN(dx);
        } else {
            b0 = SGN(dy);
        }
    }
    SH3(c0, a0);
    SH4(d0, b0);
    a1 = (dx * ((width * 40) / scr->z1)) / len;
    b1 = (dy * ((width * 40) / scr->z1)) / len;
    if (a1 == 0 && b1 == 0) {
        if (BIGGER(dx, dy)) {
            a1 = SGN(dx);
        } else {
            b1 = SGN(dy);
        }
    }
    SH3(c1, a1);
    SH4(d1, b1);
    p = (POLY_FT4 *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(p + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    p->x0 = scr->s0.v.x - d0;
    p->y0 = scr->s0.v.y + a0;
    p->x1 = scr->s0.v.x + b0;
    p->y1 = scr->s0.v.y - c0;
    p->x2 = scr->s1.v.x - d1;
    p->y2 = scr->s1.v.y + a1;
    p->x3 = scr->s1.v.x + b1;
    p->y3 = scr->s1.v.y - c1;
    *(u32 *)&p->r0 = *rgb;
    p->clut = t->clut;
    p->tpage = t->tpage;
    p->u0 = t->u;
    p->v0 = t->v;
    p->u1 = t->u + t->w;
    p->v1 = t->v;
    p->u2 = t->u;
    p->v2 = t->v + t->h;
    p->u3 = t->u + t->w;
    p->v3 = t->v + t->h;
    setlen(p, 9);
    setcode(p, 0x2E);
    n = (scr->z0 + scr->z1) >> 2;
    if (n >= 0) {
        addPrim(scr->ot + n, p);
    }
}
