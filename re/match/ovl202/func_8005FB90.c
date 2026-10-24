/* MATCHED (wave4 a6): ax/ay via the abs() builtin (fewer dx/dy refs, so scr/spr outrank dx). */
#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {                /* 0x1C: one level of the beam splitter */
    SVECTOR a;                  /* +0x00 */
    SVECTOR b;                  /* +0x08 */
    u8 v0;                      /* +0x10 texture v at a */
    u8 v1;                      /* +0x11 texture v at b */
    u8 pad12[2];
    s32 sxy;                    /* +0x14 saved end */
    s32 sz;                     /* +0x18 */
} BNode;

typedef union {
    s32 xy;
    struct { u16 x, y; } s;
} SXY;

typedef struct {
    u32 *ot;                    /* +0x00 */
    u16 depth;                  /* +0x04 */
    s16 width;                  /* +0x06 */
    u16 clut;                   /* +0x08 */
    u16 tpage;                  /* +0x0A */
    u8 u0;                      /* +0x0C */
    u8 u1;                      /* +0x0D */
    u8 pad0e[2];
    u32 col;                    /* +0x10 */
    u8 pad14[0x54];
    SXY s0;                     /* +0x68 */
    SXY s1;                     /* +0x6C */
    s32 sz0;                    /* +0x70 */
    s32 sz1;                    /* +0x74 */
    POLY_FT4 *prev;             /* +0x78 last quad, its far edge is shared */
    s32 half;                   /* +0x7C */
} BeamScr;

#define BSCR ((BeamScr *)0x1F800000)

#define SHRINKX(f, e)        \
    if ((e) <= 0) {          \
        (f) = (e);           \
        if ((f) < 0) {       \
            (f)++;           \
        }                    \
    } else {                 \
        (f) = (e) - 1;       \
    }

#define SHRINKY(f, e)        \
    if ((e) <= 0) {          \
        if ((e) < 0) {       \
            (f) = (e) + 1;   \
        } else {             \
            (f) = (e);       \
        }                    \
    } else {                 \
        (f) = (e) - 1;       \
    }

extern DrawCtx D_801A5F24;
extern s32 SquareRoot0(s32 a);
extern void func_8005F5DC(BNode *n);
extern void func_8005F2CC(SVECTOR *a, SVECTOR *b, SprInfo *spr, u32 *col);
extern s32 D_801AD694;
extern int abs(int);

typedef struct {
    u32 *ot;                    /* 0x801A5F28 */
} OtBase;
extern OtBase D_801A5F28;

#define ABS(x) ((x) < 0 ? -(x) : (x))

/* textured beam from a to b (width w): one quad when far, else the
 * recursive splitter func_8005F5DC; w == 0 also draws a plain line */
void func_8005FB90(SVECTOR *a, SVECTOR *b, SprInfo *spr, u32 *col, s32 w)
{
    BeamScr *scr;
    BNode *n;
    POLY_FT4 *p;
    s32 dx;
    s32 dy;
    s32 len;
    s32 ex0;
    s32 ey0;
    s32 fx0;
    s32 fy0;
    s32 ex1;
    s32 ey1;
    s32 fx1;
    s32 fy1;
    s32 z;
    s32 ax;
    s32 ay;

    scr = BSCR;
    if (D_801AD694 >= (a->vy + b->vy) >> 1) {
        scr->ot = D_801A5F28.ot + 120;
    } else {
        scr->ot = D_801A5F28.ot + 100;
    }
    gte_ldv0(a);
    gte_rtps();
    gte_stsxy(&scr->s0.xy);
    gte_stszotz(&scr->sz0);
    gte_ldv0(b);
    gte_rtps();
    gte_stsxy(&scr->s1.xy);
    gte_stszotz(&scr->sz1);
    if (scr->sz0 == 0 && scr->sz1 == 0) {
        return;
    }
    if (w == 0) {
        func_8005F2CC(a, b, spr, col);
    }
    if (scr->sz0 <= 320 || scr->sz1 <= 320) {
        scr->prev = 0;
        scr->depth = 2;
        scr->width = w;
        scr->clut = spr->clut;
        scr->tpage = spr->tpage;
        scr->u0 = spr->u;
        scr->u1 = spr->u + spr->w;
        n = (BNode *)((u8 *)scr + 0x14);
        n->v0 = spr->v;
        n->v1 = spr->v + (spr->h >> 1);
        scr->col = *col;
        n->a = *a;
        n->b.vx = (a->vx + b->vx) >> 1;
        n->b.vy = (a->vy + b->vy) >> 1;
        n->b.vz = (a->vz + b->vz) >> 1;
        scr->half = 0;
        func_8005F5DC(n);
        n->v0 = n->v1;
        n->v1 = spr->v + spr->h;
        n->a = n->b;
        n->b = *b;
        scr->half = 1;
        func_8005F5DC(n);
        return;
    }
    dx = (s16)scr->s1.s.x - (s16)scr->s0.s.x;
    dy = (s16)scr->s1.s.y - (s16)scr->s0.s.y;
    len = SquareRoot0(dx * dx + dy * dy);
    if (len == 0) {
        len = 1;
    }
    ex0 = (dx * ((w * 80) / scr->sz0)) / len;
    ey0 = (dy * ((w * 80) / scr->sz0)) / len;
    if (ex0 == 0 && ey0 == 0) {
        ax = abs(dx);
        ay = abs(dy);
        if (ay < ax) {
            if (dx >= 0) {
                ex0 = 1;
            } else {
                ex0 = -1;
            }
        } else {
            if (dy >= 0) {
                ey0 = 1;
            } else {
                ey0 = -1;
            }
        }
    }
    SHRINKX(fx0, ex0);
    SHRINKY(fy0, ey0);
    ex1 = (dx * ((w * 80) / scr->sz1)) / len;
    ey1 = (dy * ((w * 80) / scr->sz1)) / len;
    if (ex1 == 0 && ey1 == 0) {
        ax = abs(dx);
        ay = abs(dy);
        if (ay < ax) {
            if (dx >= 0) {
                ex1 = 1;
            } else {
                ex1 = -1;
            }
        } else {
            if (dy >= 0) {
                ey1 = 1;
            } else {
                ey1 = -1;
            }
        }
    }
    SHRINKX(fx1, ex1);
    SHRINKY(fy1, ey1);
    p = (POLY_FT4 *)D_801A5F24.buf->cur;
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    p->x0 = scr->s0.s.x - fy0;
    p->y0 = scr->s0.s.y + ex0;
    p->x1 = scr->s0.s.x + ey0;
    p->y1 = scr->s0.s.y - fx0;
    p->x2 = scr->s1.s.x - fy1;
    p->y2 = scr->s1.s.y + ex1;
    p->x3 = scr->s1.s.x + ey1;
    p->y3 = scr->s1.s.y - fx1;
    *(u32 *)&p->r0 = *col;
    p->clut = spr->clut;
    p->tpage = spr->tpage;
    p->u0 = spr->u;
    p->v0 = spr->v;
    p->u1 = spr->u + spr->w;
    p->v1 = spr->v;
    p->u2 = spr->u;
    p->v2 = spr->v + spr->h;
    p->u3 = spr->u + spr->w;
    p->v3 = spr->v + spr->h;
    setlen(p, 9);
    setcode(p, 0x2E);
    z = (scr->sz0 + scr->sz1) >> 2;
    if (z >= 0) {
        addPrim(scr->ot + z, p);
    }
}
