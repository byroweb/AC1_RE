/* ovl203 func_8006735C: draw model record idx near the camera.  Each triangle / quad
 * packet (codes 0x24 / 0x2C / 0x6C) is trivially rejected on the vertices' clip codes;
 * polygons crossing the near plane (vertex flag +7) are queued for the recursive
 * subdividers (four work lists on the stack, flat or lit by the vertex flag +0xB:
 * func_800693A4 / func_80069C8C / func_8006BA20 / func_8006C560), the rest are emitted
 * directly as flat FT3/FT4 (NCCS) or lit GT3/GT4 (func_80067324 / func_80067344). */
#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {
    s32 p00;            /* +0x00 */
    u16 unk04;          /* +0x04 */
    u16 unk06;          /* +0x06 */
    s32 p08;            /* +0x08 */
    u8 pad0c[4];
    u32 *prims;         /* +0x10 */
    u16 nprim;          /* +0x14 */
    u16 idx;            /* +0x16 */
    s32 verts;          /* +0x18 */
} V67Rec;               /* 0x1C */

typedef struct {
    u32 n;
    u32 unk04;
    u32 total;          /* +0x08 */
    V67Rec rec[1];      /* +0x0C */
} V67RecSet;

typedef struct {
    V67RecSet *model;   /* +0x000 */
    u8 pad[0x664];
    u8 verts[1];        /* +0x668 */
} V67Globals;

typedef struct {
    s32 sxy;            /* +0x00 */
    s16 sz;             /* +0x04 */
    u8 clip;            /* +0x06 clip code bits */
    u8 near;            /* +0x07 near-plane crossing */
    u8 n[3];            /* +0x08 */
    u8 lit;             /* +0x0B */
    s32 col;            /* +0x0C */
} V67Vert;

typedef struct {
    u16 *pk;
    u32 hdr;
} V67Ent;

typedef struct {
    s32 opz;            /* +0x000 */
    u8 *prim;           /* +0x004 */
    u8 *end;            /* +0x008 */
    u8 pad00c[0x350 - 0x0C];
    union {
        u32 w;
        struct {
            signed char b0;
            u8 len;
            u8 sub;
            u8 code;
        } b;
    } hdr;              /* +0x350 */
    V67Rec *rec;        /* +0x354 */
    s32 nbase;          /* +0x358 */
    s32 unk35c;         /* +0x35C */
    u8 pad360[8];
    s32 count;          /* +0x368 */
    u32 *ot;            /* +0x36C */
    u8 pad370[8];
    s32 sh3;            /* +0x378 */
    s32 sh4;            /* +0x37C */
    V67Ent *l0;         /* +0x380 */
    V67Ent *l1;         /* +0x384 */
    V67Ent *l2;         /* +0x388 */
    V67Ent *l3;         /* +0x38C */
    u8 r, g, b;         /* +0x390 */
} V67Scr;

typedef struct {        /* POLY_FT3 / POLY_FT4 */
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;            /* +0x08 */
    u32 uv0;            /* +0x0C */
    s32 xy1;            /* +0x10 */
    u32 uv1;            /* +0x14 */
    s32 xy2;            /* +0x18 */
    u16 uv2;            /* +0x1C */
    u16 pad1e;
    s32 xy3;            /* +0x20 */
    u16 uv3;            /* +0x24 */
    u16 pad26;
} V67FT;

typedef struct {        /* POLY_GT3 / POLY_GT4 */
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
} V67GT;

typedef struct {
    PrimBuf *buf;
} V67Ctx;

extern V67Globals D_8019D968;
extern u32 *D_8019D960;
extern V67Ctx D_8019D95C;
extern s32 D_80194730;
extern s32 D_8019DF7C;

extern void func_80067080(s32 n);
extern s32 func_800672F4(s32 a, s32 b, s32 c);
extern void func_80067324(s32 n, u8 *v);
extern void func_80067344(u8 *v);
extern void func_800693A4(V67Ent *l);
extern void func_80069C8C(V67Ent *l);
extern void func_8006BA20(V67Ent *l);
extern void func_8006C560(V67Ent *l);

#define SCR ((V67Scr *)0x1F800000)
#define VTX(off) ((V67Vert *)(verts + (off)))
#define FT ((V67FT *)p)
#define GT ((V67GT *)p)

void func_8006735C(u16 idx)
{
    V67Ent b0[256];
    V67Ent b1[256];
    V67Ent b2[256];
    V67Ent b3[256];
    V67Scr *s;
    u16 *pk;
    u8 *verts;
    V67Vert *a;
    V67Vert *b;
    V67Vert *c;
    V67Vert *d;
    u8 *p;
    u32 t;
    s32 k;
    s32 z;

    s = SCR;
    SCR->rec = (V67Rec *)((u8 *)D_8019D968.model + 12 + idx * 32 - idx * 4);
    if (SCR->rec->nprim == 0)
        return;
    SCR->count = SCR->rec->nprim;
    SCR->b = 64;
    SCR->g = 64;
    SCR->r = 64;
    SCR->l0 = b0;
    SCR->l1 = b1;
    SCR->l2 = b2;
    SCR->l3 = b3;
    SCR->sh3 = D_80194730 + 15;
    SCR->sh4 = D_80194730 + 3;
    SCR->ot = D_8019D960 + 120;
    verts = D_8019D968.verts;
    func_80067080(SCR->rec->unk04);
    SCR->nbase = SCR->rec->p08;
    SCR->unk35c = D_8019DF7C;
    pk = (u16 *)SCR->rec->prims;
    SCR->prim = D_8019D95C.buf->cur;
    SCR->end = D_8019D95C.buf->end;
    do {
        s->hdr.w = *(u32 *)pk;
        pk += 2;
        switch (s->hdr.b.code & 0xFC) {
        case 0x24:
            a = VTX(pk[7]);
            b = VTX(pk[8]);
            c = VTX(pk[9]);
            if (c->clip & (a->clip & b->clip))
                break;
            t = c->near | (a->near | b->near);
            if (t != 0)
                goto tsub;
        tplain:
            if ((c->clip | (a->clip | b->clip)) == 31)
                break;
            if (func_800672F4(a->sxy, b->sxy, c->sxy) <= 0)
                break;
            if ((c->lit | (a->lit | b->lit)) == 0) {
                gte_ldv0(s->nbase + pk[6]);
                gte_ldrgb(&s->r);
                gte_nccs();
                p = s->prim;
                s->prim = p + 32;
                FT->xy0 = a->sxy;
                FT->xy1 = b->sxy;
                FT->xy2 = c->sxy;
                FT->uv0 = ((u32 *)pk)[0];
                FT->uv1 = ((u32 *)pk)[1];
                FT->uv2 = pk[4];
                setlen(p, 7);
                gte_strgb(&FT->r0);
                FT->code = s->hdr.b.code;
                z = (((a->sz + b->sz + c->sz) * 0x1555) >> s->sh3) + (s->hdr.b.b0 << 2);
                addPrim(s->ot + z, p);
            } else {
                func_80067324(s->nbase + pk[6], a->n);
                p = s->prim;
                s->prim = p + 40;
                gte_strgb(&GT->r0);
                func_80067344(b->n);
                GT->xy0 = a->sxy;
                GT->xy1 = b->sxy;
                GT->xy2 = c->sxy;
                gte_strgb(GT->c1);
                func_80067344(c->n);
                GT->uv0 = ((u32 *)pk)[0];
                GT->uv1 = ((u32 *)pk)[1];
                GT->uv2 = pk[4];
                setlen(p, 9);
                GT->code = (s->hdr.b.code & 2) | 0x34;
                gte_strgb(GT->c2);
                z = (((a->sz + b->sz + c->sz) * 0x1555) >> s->sh3) + (s->hdr.b.b0 << 2);
                addPrim(s->ot + z, p);
            }
            break;
        tsub:
            if (t < 2) {
                k = s->hdr.b.sub & 7;
                if (k == 0)
                    goto tplain;
                if (func_800672F4(a->sxy, b->sxy, c->sxy) <= 0)
                    break;
            } else {
                k = s->hdr.b.sub >> 3;
                if (k == 0)
                    goto tplain;
            }
            if ((c->lit | (a->lit | b->lit)) == 0) {
                s->l1->pk = pk;
                s->hdr.b.sub = k;
                s->l1->hdr = s->hdr.w;
                s->l1++;
            } else {
                s->l3->pk = pk;
                s->hdr.b.sub = k;
                s->l3->hdr = s->hdr.w;
                s->l3++;
            }
            break;
        case 0x2C:
            a = VTX(pk[9]);
            b = VTX(pk[10]);
            c = VTX(pk[11]);
            d = VTX(pk[12]);
            if (d->clip & (c->clip & (a->clip & b->clip)))
                break;
            t = d->near | (c->near | (a->near | b->near));
            if (t != 0)
                goto qsub;
        qplain:
            if ((d->clip | (c->clip | (a->clip | b->clip))) == 31)
                break;
            if (func_800672F4(a->sxy, b->sxy, c->sxy) <= 0)
                break;
            if ((d->lit | (c->lit | (a->lit | b->lit))) != 0)
                goto qlit;
            gte_ldv0(s->nbase + pk[8]);
            gte_ldrgb(&s->r);
            gte_nccs();
            p = s->prim;
            s->prim = p + 40;
            FT->xy0 = a->sxy;
            FT->xy1 = b->sxy;
            FT->xy2 = c->sxy;
            FT->xy3 = d->sxy;
            FT->uv0 = ((u32 *)pk)[0];
            FT->uv1 = ((u32 *)pk)[1];
            FT->uv2 = pk[4];
            FT->uv3 = pk[6];
            setlen(p, 9);
            gte_strgb(&FT->r0);
            FT->code = s->hdr.b.code;
            z = ((a->sz + b->sz + c->sz + d->sz) >> s->sh4) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, p);
            break;
        qlit:
            func_80067324(s->nbase + pk[8], a->n);
            p = s->prim;
            s->prim = p + 52;
            gte_strgb(&GT->r0);
            func_80067344(b->n);
            GT->xy0 = a->sxy;
            GT->xy1 = b->sxy;
            GT->xy2 = c->sxy;
            GT->xy3 = d->sxy;
            gte_strgb(GT->c1);
            func_80067344(c->n);
            GT->uv0 = ((u32 *)pk)[0];
            GT->uv1 = ((u32 *)pk)[1];
            GT->uv2 = pk[4];
            GT->uv3 = pk[6];
            gte_strgb(GT->c2);
            func_80067344(d->n);
            setlen(p, 12);
            GT->code = (s->hdr.b.code & 2) | 0x3C;
            gte_strgb(GT->c3);
            z = ((a->sz + b->sz + c->sz + d->sz) >> s->sh4) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, p);
            break;
        qsub:
            if (t < 2) {
                k = s->hdr.b.sub & 7;
                if (k == 0)
                    goto qplain;
                if (func_800672F4(a->sxy, b->sxy, c->sxy) <= 0)
                    break;
            } else {
                k = s->hdr.b.sub >> 3;
                if (k == 0)
                    goto qplain;
            }
            if ((d->lit | (c->lit | (a->lit | b->lit))) == 0)
                goto push0;
            goto push2;
        case 0x6C:
            a = VTX(pk[9]);
            b = VTX(pk[10]);
            c = VTX(pk[11]);
            d = VTX(pk[12]);
            if (d->clip & (c->clip & (a->clip & b->clip)))
                break;
            t = d->near | (c->near | (a->near | b->near));
            if (t != 0)
                goto wsub;
        wplain:
            if ((d->clip | (c->clip | (a->clip | b->clip))) == 31)
                break;
            if ((d->lit | (c->lit | (a->lit | b->lit))) != 0)
                goto qlit;
            gte_ldv0(s->nbase + pk[8]);
            gte_ldrgb(&s->r);
            gte_nccs();
            p = s->prim;
            s->prim = p + 40;
            FT->xy0 = a->sxy;
            FT->xy1 = b->sxy;
            FT->xy2 = c->sxy;
            FT->xy3 = d->sxy;
            FT->uv0 = ((u32 *)pk)[0];
            FT->uv1 = ((u32 *)pk)[1];
            FT->uv2 = pk[4];
            FT->uv3 = pk[6];
            setlen(p, 9);
            gte_strgb(&FT->r0);
            FT->code = s->hdr.b.code & 0xBF;
            z = ((a->sz + b->sz + c->sz + d->sz) >> s->sh4) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, p);
            break;
        wsub:
            if (t < 2)
                k = s->hdr.b.sub & 7;
            else
                k = s->hdr.b.sub >> 3;
            if (k == 0)
                goto wplain;
            if ((d->lit | (c->lit | (a->lit | b->lit))) == 0) {
            push0:
                s->l0->pk = pk;
                s->hdr.b.sub = k;
                s->l0->hdr = s->hdr.w;
                s->l0++;
            } else {
            push2:
                s->l2->pk = pk;
                s->hdr.b.sub = k;
                s->l2->hdr = s->hdr.w;
                s->l2++;
            }
            break;
        }
        pk += s->hdr.b.len * 2;
    } while (--s->count != 0);
    s->sh3 -= 14;
    s->l1->pk = 0;
    func_800693A4(b1);
    s->l0->pk = 0;
    func_80069C8C(b0);
    s->l3->pk = 0;
    func_8006BA20(b3);
    s->l2->pk = 0;
    func_8006C560(b2);
    D_8019D95C.buf->cur = s->prim;
}
