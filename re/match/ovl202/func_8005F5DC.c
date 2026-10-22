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

/* textured beam a-b of screen width BSCR->width, halved recursively near
 * the camera like func_8005EF38; consecutive quads share their edge */
void func_8005F5DC(BNode *n)
{
    BeamScr *scr;
    BNode *c;
    POLY_FT4 *p;
    s32 dx;
    s32 dy;
    s32 len;
    s32 w;

    s32 ex;
    s32 ey;
    s32 fx;
    s32 fy;
    s32 a;
    s32 b;
    s32 z;

    scr = BSCR;
    if (BSCR->half == 0) {
        n->sxy = BSCR->s1.xy;
        n->sz = BSCR->sz1;
        gte_ldv0(&n->b);
        gte_rtps();
        gte_stsxy(&BSCR->s1.xy);
        gte_stszotz(&BSCR->sz1);
    } else {
        BSCR->s0.xy = BSCR->s1.xy;
        BSCR->sz0 = BSCR->sz1;
        a = n->sxy;
        b = n->sz;
        BSCR->s1.xy = a;
        BSCR->sz1 = b;
    }
    if (scr->sz0 <= 320 || scr->sz1 <= 320) {
        if (scr->depth == 0) {
            if (scr->sz0 >= 16 && scr->sz1 >= 16) {
                goto draw;
            }
            return;
        } else {
            scr->depth--;
            c = n + 1;
            c->a = n->a;
            c->b.vx = (n->a.vx + n->b.vx) >> 1;
            c->b.vy = (n->a.vy + n->b.vy) >> 1;
            c->b.vz = (n->a.vz + n->b.vz) >> 1;
            c->v0 = n->v0;
            c->v1 = (n->v0 + n->v1) >> 1;
            scr->half = 0;
            func_8005F5DC(c);
            c->a = c->b;
            c->b = n->b;
            c->v0 = c->v1;
            c->v1 = n->v1;
            scr->half = 1;
            func_8005F5DC(c);
            scr->depth++;
            return;
        }
    }
draw:
    dx = (s16)scr->s1.s.x - (s16)scr->s0.s.x;
    dy = (s16)scr->s1.s.y - (s16)scr->s0.s.y;
    len = SquareRoot0(dx * dx + dy * dy);
    if (len == 0) {
        len = 1;
    }
    p = (POLY_FT4 *)D_801A5F24.buf->cur;
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    if (scr->prev == 0) {
        s32 ax, ay, bx, by;
        ax = (dx * ((scr->width * 80) / scr->sz0)) / len;
        ay = (dy * ((scr->width * 80) / scr->sz0)) / len;
        SHRINKX(bx, ax);
        SHRINKY(by, ay);
        p->x0 = scr->s0.s.x - by;
        p->y0 = scr->s0.s.y + ax;
        p->x1 = scr->s0.s.x + ay;
        p->y1 = scr->s0.s.y - bx;
    } else {
        *(s32 *)&p->x0 = *(s32 *)&scr->prev->x2;
        *(s32 *)&p->x1 = *(s32 *)&scr->prev->x3;
    }
    w = (scr->width * 80) / scr->sz1;
    ex = (dx * w) / len;
    ey = (dy * w) / len;
    scr->prev = p;
    SHRINKX(fx, ex);
    SHRINKY(fy, ey);
    p->x2 = scr->s1.s.x - fy;
    p->y2 = scr->s1.s.y + ex;
    p->x3 = scr->s1.s.x + ey;
    p->y3 = scr->s1.s.y - fx;
    *(u32 *)&p->r0 = scr->col;
    p->clut = scr->clut;
    p->tpage = scr->tpage;
    p->u0 = p->u2 = scr->u0;
    p->u1 = p->u3 = scr->u1;
    p->v0 = p->v1 = n->v0;
    p->v2 = p->v3 = n->v1;
    setlen(p, 9);
    setcode(p, 0x2E);
    z = (scr->sz0 + scr->sz1) >> 2;
    if (z >= 0) {
        addPrim(scr->ot + z, p);
    }
}
