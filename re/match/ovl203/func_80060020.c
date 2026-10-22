#include "ac1/ac1.h"
#include "ac1/gte.h"
#include "ac1/prim2d.h"

#define gte_rtv0() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A480012")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25, 0(%0)\n\t" \
    "swc2 $26, 4(%0)\n\t" \
    "swc2 $27, 8(%0)" : : "r"(r0) : "memory")

typedef struct {                /* scratchpad 0x1F800000 */
    u32 *ot;                    /* +0x00 */
    u8 pad04[8];
    MATRIX m;                   /* +0x0C */
    SVECTOR v;                  /* +0x2C */
} BbScr;

typedef struct {
    u32 uv0;                    /* +0x00 uv0 + clut */
    u32 uv1;                    /* +0x04 uv1 + tpage */
    u16 uv2;                    /* +0x08 */
    u16 tri;                    /* +0x0A 0 = quad */
    u16 uv3;                    /* +0x0C */
} BbTex;

typedef struct {
    u32 tag;
    u32 rgbc;                   /* +0x04 */
    s32 xy0;                    /* +0x08 */
    u32 uv0;                    /* +0x0C */
    s32 xy1;                    /* +0x10 */
    u32 uv1;                    /* +0x14 */
    s32 xy2;                    /* +0x18 */
    u16 uv2;                    /* +0x1C */
    u16 pad1e;
    s32 xy3;                    /* +0x20 */
    u16 uv3;                    /* +0x24 */
    u16 pad26;
} PolyFT4w;

typedef struct {
    PrimBuf *buf;               /* 0x8019D95C */
    u32 *ot;                    /* 0x8019D960 */
    u8 pad08[0x72A0];
    MATRIX cam;                 /* 0x801A4C04 */
} RenderCtx;

extern RenderCtx D_8019D95C;
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern s32 func_8005FFE4(SVECTOR *v, s32 *xy);

void func_80060020(SVECTOR *pos, SVECTOR *rot, BbTex *tx, s32 size)
{
    BbScr *s;
    PolyFT4w *p;
    PolyFT4w *q;
    s32 z;

    s = (BbScr *)0x1F800000;
    s->ot = D_8019D95C.ot + 100;
    gte_ldv0(pos);
    gte_rtv0();
    gte_stlvnl(s->m.t);
    RotMatrixYXZ(rot, &s->m);
    MulMatrix2(&D_8019D95C.cam, &s->m);
    SetRotMatrix(&s->m);
    SetTransMatrix(&s->m);
    if (tx->tri == 0) {
        p = (PolyFT4w *)D_8019D95C.buf->cur;
        D_8019D95C.buf->cur += 0x28;
        if (D_8019D95C.buf->cur > D_8019D95C.buf->end) {
            return;
        }
        p->uv0 = tx->uv0;
        p->uv1 = tx->uv1;
        p->uv2 = tx->uv2;
        p->uv3 = tx->uv3;
        p->rgbc = 0x808080;
        s->v.vz = 0;
        s->v.vx = s->v.vy = -size;
        z = func_8005FFE4(&s->v, &p->xy0) >> 1;
        s->v.vx = size;
        func_8005FFE4(&s->v, &p->xy1);
        s->v.vy = size;
        func_8005FFE4(&s->v, &p->xy3);
        s->v.vx = -size;
        func_8005FFE4(&s->v, &p->xy2);
        ((u8 *)p)[3] = 9;
        ((u8 *)p)[7] = 0x2C;
        if (z >= 0) {
            addPrim(s->ot + z, p);
        }
    } else {
        q = (PolyFT4w *)D_8019D95C.buf->cur;
        D_8019D95C.buf->cur += 0x20;
        if (D_8019D95C.buf->cur > D_8019D95C.buf->end) {
            return;
        }
        q->uv0 = tx->uv0;
        q->uv1 = tx->uv1;
        q->uv2 = tx->uv2;
        q->rgbc = 0x808080;
        s->v.vz = 0;
        s->v.vx = 0;
        s->v.vy = -size;
        z = func_8005FFE4(&s->v, &q->xy0) >> 1;
        s->v.vx = -size;
        s->v.vy = size;
        func_8005FFE4(&s->v, &q->xy1);
        s->v.vx = size;
        func_8005FFE4(&s->v, &q->xy2);
        ((u8 *)q)[3] = 7;
        ((u8 *)q)[7] = 0x24;
        if (z >= 0) {
            addPrim(s->ot + z, q);
        }
    }
}
