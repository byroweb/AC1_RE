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
    u8 unk14[0x68 - 0x14];
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
#define SH1(y, x) if ((x) <= 0) { if ((x) < 0) { y = (x) + 1; } else { y = (x); } } else { y = (x) - 1; }
#define SH2(y, x) if ((x) <= 0) { if ((x) < 0) { y = (x) + 1; } else { y = (x); } } else { y = (x) - 1; }
#define SH3(y, x) if ((x) <= 0) { if ((x) < 0) { y = (x) + 1; } else { y = (x); } } else { y = (x) - 1; }
#define SH4(y, x) if ((x) <= 0) { if ((x) < 0) { y = (x) + 1; } else { y = (x); } } else { y = (x) - 1; }

extern DrawCtx D_801ADCCC;
extern s32 SquareRoot0(s32 a);

/* recursive textured ribbon: like func_8005DEE4 but emits a POLY_FT4 strip of
 * screen width SCR->width, chained through SCR->prev */
void func_8005E588(RibNode *nd)
{
    RibNode *nx;
    POLY_FT4 *p;
    s32 n;
    s32 dx, dy, len, w, a, b, c, d;
    s32 a0, b0, c0, d0;
    RibScratch *scr = SCR;

    if (SCR->half == 0) {
        nd->sxy = SCR->s1.xy;
        nd->z = SCR->z1;
        gte_ldv0(&nd->b);
        gte_rtps();
        gte_stsxy(0x1F80006C);
        gte_stszotz(0x1F800074);
    } else {
        SCR->s0.xy = SCR->s1.xy;
        SCR->z0 = SCR->z1;
        {
            s32 xy = nd->sxy;
            s32 z = nd->z;
            SCR->s1.xy = xy;
            SCR->z1 = z;
        }
    }
    if (scr->z0 <= 160 || scr->z1 <= 160) {
        if (scr->depth == 0) {
            if (scr->z0 >= 8 && scr->z1 >= 8) {
                goto draw;
            }
        } else {
            scr->depth--;
            nx = nd + 1;
            nx->a = nd->a;
            nx->b.vx = (nd->a.vx + nd->b.vx) >> 1;
            nx->b.vy = (nd->a.vy + nd->b.vy) >> 1;
            nx->va = nd->va;
            nx->b.vz = (nd->a.vz + nd->b.vz) >> 1;
            nx->vb = (nd->va + nd->vb) >> 1;
            scr->half = 0;
            func_8005E588(nx);
            nx->a = nx->b;
            nx->b = nd->b;
            nx->va = nx->vb;
            nx->vb = nd->vb;
            scr->half = 1;
            func_8005E588(nx);
            scr->depth++;
        }
        return;
    }
draw:
    dx = scr->s1.v.x - scr->s0.v.x;
    dy = scr->s1.v.y - scr->s0.v.y;
    len = SquareRoot0(dx * dx + dy * dy);
    if (len == 0) {
        len = 1;
    }
    p = (POLY_FT4 *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(p + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    if (scr->prev == 0) {
        a0 = (dx * ((scr->width * 40) / scr->z0)) / len;
        b0 = (dy * ((scr->width * 40) / scr->z0)) / len;
        SH1(c0, a0);
        SH2(d0, b0);
        p->x0 = scr->s0.v.x - d0;
        p->y0 = scr->s0.v.y + a0;
        p->x1 = scr->s0.v.x + b0;
        p->y1 = scr->s0.v.y - c0;
    } else {
        *(s32 *)&p->x0 = *(s32 *)&scr->prev->x2;
        *(s32 *)&p->x1 = *(s32 *)&scr->prev->x3;
    }
    w = scr->width;
    w = (w * 40) / scr->z1;
    a = (dx * w) / len;
    b = (dy * w) / len;
    scr->prev = p;
    SH3(c, a);
    SH4(d, b);
    p->x2 = scr->s1.v.x - d;
    p->y2 = scr->s1.v.y + a;
    p->x3 = scr->s1.v.x + b;
    p->y3 = scr->s1.v.y - c;
    *(u32 *)&p->r0 = scr->rgb;
    p->clut = scr->clut;
    p->tpage = scr->tpage;
    p->u0 = p->u2 = scr->u0;
    p->u1 = p->u3 = scr->u1;
    p->v0 = p->v1 = nd->va;
    p->v2 = p->v3 = nd->vb;
    setlen(p, 9);
    setcode(p, 0x2E);
    n = (scr->z0 + scr->z1) >> 2;
    if (n >= 0) {
        addPrim(scr->ot + n, p);
    }
}
