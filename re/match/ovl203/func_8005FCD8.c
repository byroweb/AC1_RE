#include "ac1/ac1.h"
#include "ac1/gte.h"
#include "ac1/prim2d.h"

#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 0(%1)\n\tswc2 $14, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")

typedef struct {                /* scratchpad 0x1F800000 */
    u32 *ot;                    /* +0x00 */
    union {
        SVECTOR v[4];           /* +0x04 corners */
        struct {
            u8 pad[0x0C];
            MATRIX m;           /* +0x10 */
            u8 pad30[8];
            s32 flag;           /* +0x38 */
        } t;
    } u;
} SprScr;

typedef struct {
    u16 uv0clut;                /* +0x00 */
    u16 tpage;                  /* +0x02 */
    u8 u, v;                    /* +0x04 */
    u8 w, h;                    /* +0x06 */
    u32 rgbc;                   /* +0x08 */
} SprTex;

typedef struct {
    u32 tag;
    u32 rgbc;                   /* +0x04 */
    s32 xy0;                    /* +0x08 */
    u8 u0, v0;                  /* +0x0C */
    u16 clut;                   /* +0x0E */
    s32 xy1;                    /* +0x10 */
    u8 u1, v1;                  /* +0x14 */
    u16 tpage;                  /* +0x16 */
    s32 xy2;                    /* +0x18 */
    u8 u2, v2;                  /* +0x1C */
    u16 pad1e;
    s32 xy3;                    /* +0x20 */
    u8 u3, v3;                  /* +0x24 */
    u16 pad26;
} PolyFT4b;

typedef struct {
    PrimBuf *buf;               /* 0x8019D95C */
    u32 *ot;                    /* 0x8019D960 */
    u8 pad08[0x73F0];
    MATRIX rot;                 /* 0x801A4D54 */
} RenderCtx;

extern RenderCtx D_8019D95C;
extern long RotTrans(SVECTOR *v0, VECTOR *v1, long *flag);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);

void func_8005FCD8(SVECTOR *pos, SprTex *tx, s32 w, s32 h, s32 ot)
{
    SprScr *s;
    PolyFT4b *p;
    s32 z;

    s = (SprScr *)0x1F800000;
    s->ot = D_8019D95C.ot + ot;
    RotTrans(pos, (VECTOR *)s->u.t.m.t, &s->u.t.flag);
    SetTransMatrix(&s->u.t.m);
    SetRotMatrix(&D_8019D95C.rot);
    p = (PolyFT4b *)D_8019D95C.buf->cur;
    D_8019D95C.buf->cur += 0x28;
    if (D_8019D95C.buf->cur > D_8019D95C.buf->end) {
        return;
    }
    s->u.v[0].vx = s->u.v[2].vx = -w;
    s->u.v[1].vx = s->u.v[3].vx = w;
    s->u.v[0].vy = s->u.v[1].vy = -h;
    s->u.v[0].vz = s->u.v[1].vz = s->u.v[2].vz = s->u.v[3].vz = 0;
    s->u.v[2].vy = s->u.v[3].vy = 0;
    gte_ldv3(&s->u.v[0], &s->u.v[1], &s->u.v[2]);
    gte_rtpt();
    p->u0 = tx->u;
    p->v0 = tx->v;
    p->u1 = tx->u + tx->w;
    p->v1 = tx->v;
    p->u2 = tx->u;
    p->v2 = tx->v + tx->h;
    p->u3 = tx->u + tx->w;
    p->v3 = tx->v + tx->h;
    gte_stszotz(&z);
    if (z != 0) {
        gte_stsxy3(&p->xy0, &p->xy1, &p->xy2);
        gte_ldv0(&s->u.v[3]);
        gte_rtps();
        p->rgbc = tx->rgbc;
        p->clut = tx->uv0clut;
        p->tpage = tx->tpage;
        gte_stsxy(&p->xy3);
        ((u8 *)p)[3] = 9;
        ((u8 *)p)[7] = 0x2E;
        z >>= 1;
        if (z >= 0) {
            addPrim(s->ot + z, p);
        }
    } else {
        D_8019D95C.buf->cur -= 0x28;
    }
}
