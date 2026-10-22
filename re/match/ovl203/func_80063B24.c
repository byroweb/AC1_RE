/* ovl203 func_80063B24: draw one model record (idx) as lit gouraud-textured triangles and
 * quads (packet codes 0x24 / 0x2C / 0x6C) into the ordering table at D_8019D960 + otofs,
 * with back-face culling (func_80063AC8) and per-vertex lighting (func_80063A88 /
 * func_80063AAC + GTE RGB stores).  Scratchpad state at 0x1F800000. */
#include "ac1/ac1.h"

#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {
    s32 p00;            /* +0x00 */
    u16 unk04;          /* +0x04 */
    u8 pad06[2];
    s32 p08;            /* +0x08 */
    u8 pad0c[4];
    u32 *prims;         /* +0x10 */
    u16 nprim;          /* +0x14 */
    u16 idx;            /* +0x16 */
    s32 verts;          /* +0x18 */
} V63Rec;               /* 0x1C */

typedef struct {
    u32 n;
    u32 unk04;
    u32 total;          /* +0x08 */
    V63Rec rec[1];      /* +0x0C */
} V63RecSet;

typedef struct {
    V63RecSet *model;   /* +0x000 */
    u8 pad[0x664];
    u8 verts[1];        /* +0x668 */
} V63Globals;

typedef struct {
    s32 sxy;            /* +0x00 */
    s16 sz;             /* +0x04 */
    u8 pad6[2];
    u8 n[4];            /* +0x08 */
    s32 col;            /* +0x0C */
} V63Vert;

typedef struct {
    s32 opz;            /* +0x00 */
    u8 *prim;           /* +0x04 */
    u8 *end;            /* +0x08 */
    union {
        u32 w;
        struct {
            signed char b0;
            u8 len;
            u8 b2;
            u8 code;
        } b;
    } hdr;              /* +0x0C */
    V63Rec *rec;        /* +0x10 */
    s32 nbase;          /* +0x14 */
    s32 unk18;          /* +0x18 */
    s32 count;          /* +0x1C */
    u32 *ot;            /* +0x20 */
    s32 sh3;            /* +0x24 */
    s32 sh4;            /* +0x28 */
    u8 r, g, b;         /* +0x2C */
} V63Scr;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;
    u32 uv0;            /* +0x0C */
    u8 c1[4];           /* +0x10 */
    s32 xy1;            /* +0x14 */
    u32 uv1;            /* +0x18 */
    u8 c2[4];           /* +0x1C */
    s32 xy2;            /* +0x20 */
    u16 uv2;            /* +0x24 */
    u16 pad26;
} V63GT3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;
    u32 uv0;            /* +0x0C */
    u8 c1[4];           /* +0x10 */
    s32 xy1;            /* +0x14 */
    u32 uv1;            /* +0x18 */
    u8 c2[4];           /* +0x1C */
    s32 xy2;            /* +0x20 */
    u16 uv2;            /* +0x24 */
    u16 pad26;
    u8 c3[4];           /* +0x28 */
    s32 xy3;            /* +0x2C */
    u16 uv3;            /* +0x30 */
    u16 pad32;
} V63GT4;

typedef struct {
    PrimBuf *buf;
} V63Ctx;

extern V63Globals D_8019D968;
extern V63Ctx D_8019D95C;
extern u32 *D_8019D960;
extern s32 D_80194734;
extern s32 D_801A4BEC;
extern s32 D_8019DF7C;

extern void func_80064088(s32 a, s32 b, V63Rec *r);
extern void func_80063034(s32 a, s32 b, V63Rec *r);
extern void func_80063AC8(V63Scr *s, s32 a, s32 b, s32 c);
extern void func_80063A88(s32 n, u8 *v, s32 col);
extern void func_80063AAC(u8 *v, s32 col);

#define addPrimM(ot, p)     (*(u32 *)(p) = (*(u32 *)(p) & 0xFF000000) | (*(u32 *)(ot) & 0x00FFFFFF),      *(u32 *)(ot) = (*(u32 *)(ot) & 0xFF000000) | ((u32)(p) & 0x00FFFFFF))
#define VTX(off) ((V63Vert *)(verts + (off)))

void func_80063B24(u16 idx, s32 otofs)
{
    V63Scr *s;
    V63Rec *r;
    u16 *pk;
    u8 *verts;
    V63Vert *a;
    V63Vert *b;
    V63Vert *c;
    V63Vert *d;
    V63GT3 *p3;
    V63GT4 *p4;
    s32 z;

    s = (V63Scr *)0x1F800000;
    s->rec = r = (V63Rec *)((u8 *)D_8019D968.model + 12 + idx * 32 - idx * 4);
    if (r->nprim == 0)
        return;
    s->count = r->nprim;
    s->b = 64;
    s->g = 64;
    s->r = 64;
    s->sh3 = D_80194734 + 15;
    s->sh4 = D_80194734 + 3;
    s->ot = D_8019D960 + otofs;
    verts = D_8019D968.verts;
    if (D_801A4BEC == 0)
        func_80064088(r->p00, r->unk04, r);
    else
        func_80063034(r->p00, r->unk04, r);
    s->nbase = s->rec->p08;
    s->unk18 = D_8019DF7C;
    pk = (u16 *)s->rec->prims;
    s->prim = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    do {
        s->hdr.w = *(u32 *)pk;
        pk += 2;
        switch (s->hdr.b.code & 0xFC) {
        case 0x24:
            a = VTX(pk[7]);
            b = VTX(pk[8]);
            c = VTX(pk[9]);
            func_80063AC8(s, a->sxy, b->sxy, c->sxy);
            if (s->opz <= 0)
                break;
            func_80063A88(s->nbase + pk[6], a->n, a->col);
            p4 = (V63GT4 *)s->prim;
            s->prim = (u8 *)p4 + sizeof(V63GT3);
            gte_strgb(&p4->r0);
            func_80063AAC(b->n, b->col);
            p4->xy0 = a->sxy;
            p4->xy1 = b->sxy;
            p4->xy2 = c->sxy;
            gte_strgb(p4->c1);
            func_80063AAC(c->n, c->col);
            p4->uv0 = ((u32 *)pk)[0];
            p4->uv1 = ((u32 *)pk)[1];
            p4->uv2 = pk[4];
            setlen(p4, 9);
            p4->code = (s->hdr.b.code & 2) | 0x34;
            z = (((a->sz + b->sz + c->sz) * 0x1555) >> s->sh3) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, p4);
            gte_strgb(p4->c2);
            break;
        case 0x2C:
            a = VTX(pk[9]);
            b = VTX(pk[10]);
            c = VTX(pk[11]);
            func_80063AC8(s, a->sxy, b->sxy, c->sxy);
            if (s->opz <= 0)
                break;
            func_80063A88(s->nbase + pk[8], a->n, a->col);
        gt4:
            p4 = (V63GT4 *)s->prim;
            d = VTX(pk[12]);
            s->prim = (u8 *)(p4 + 1);
            gte_strgb(&p4->r0);
            func_80063AAC(b->n, b->col);
            p4->xy0 = a->sxy;
            p4->xy1 = b->sxy;
            p4->xy2 = c->sxy;
            p4->xy3 = d->sxy;
            gte_strgb(p4->c1);
            func_80063AAC(c->n, c->col);
            p4->uv0 = ((u32 *)pk)[0];
            p4->uv1 = ((u32 *)pk)[1];
            p4->uv2 = pk[4];
            p4->uv3 = pk[6];
            gte_strgb(p4->c2);
            func_80063AAC(d->n, d->col);
            setlen(p4, 12);
            p4->code = (s->hdr.b.code & 2) | 0x3C;
            z = ((a->sz + b->sz + c->sz + d->sz) >> s->sh4) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, p4);
            gte_strgb(p4->c3);
            break;
        case 0x6C:
            a = VTX(pk[9]);
            func_80063A88(s->nbase + pk[8], a->n, a->col);
            b = VTX(pk[10]);
            c = VTX(pk[11]);
            goto gt4;
        }
        pk += s->hdr.b.len * 2;
    } while (--s->count != 0);
    D_8019D95C.buf->cur = s->prim;
}
