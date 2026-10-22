/* ovl203 func_8006252C: draw one model record as textured polygons with a light colour
 * table (lt): flat-lit FT3/FT4 (packet codes 0x24 / 0x2C / 0x6C, colour from
 * func_8006250C) and per-vertex lit GT3/GT4 (0x25 / 0x2D, GTE NCDS with the record's
 * normals), back-face culled by func_800624E8, into the OT D_8019D960 + otofs.
 * Scratchpad state at 0x1F800000.  Called by func_80062E78 for depth-cued parts. */
#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {
    s32 p00;            /* +0x00 */
    u16 unk04;          /* +0x04 */
    u8 pad06[2];
    s32 p08;            /* +0x08 */
    u8 pad0c[4];
    u32 *prims;         /* +0x10 */
    u16 nprim;          /* +0x14 */
} V62Rec;

typedef struct {
    u32 *ot;            /* +0x000 */
    u8 pad[0x66C];
    u8 verts[1];        /* +0x670 */
} V62Globals;

typedef struct {
    s32 sxy;            /* +0x00 */
    s16 sz;             /* +0x04 */
    u8 pad6[2];
    u8 n[4];            /* +0x08 */
    s32 col;            /* +0x0C */
} V62Vert;

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
    s32 nbase;          /* +0x10 */
    s32 vtab;           /* +0x14 */
    s32 count;          /* +0x18 */
    u32 *ot;            /* +0x1C */
    s32 sh3;            /* +0x20 */
    s32 sh4;            /* +0x24 */
    u8 *col;            /* +0x28 */
    u8 r, g, b;         /* +0x2C */
} V62Scr;

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
} V62FT;

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
} V62GT;

typedef struct {
    PrimBuf *buf;
} V62Ctx;

extern V62Globals D_8019D960;
extern V62Ctx D_8019D95C;
extern s32 D_80194734;
extern u8 D_8008C990[][128];

extern void func_8006249C(s32 a, s32 b);
extern void func_800624E8(V62Scr *s, s32 a, s32 b, s32 c);
extern void func_8006250C(s32 n, u8 *rgb);

#define VTX(off) ((V62Vert *)(verts + (off)))
#define IDX(off) (*(s16 *)(s->vtab + ((off) >> 1) + 6))

void func_8006252C(V62Rec *r, s32 lt, s32 otofs)
{
    V62Scr *s;
    u16 *pk;
    u8 *verts;
    V62Vert *a;
    V62Vert *b;
    V62Vert *c;
    V62Vert *d;
    V62FT *pf;
    V62GT *pg;
    s32 z;

    s = (V62Scr *)0x1F800000;
    s->count = r->nprim;
    if (r->nprim == 0)
        return;
    s->b = 64;
    s->g = 64;
    s->r = 64;
    s->sh3 = D_80194734 + 15;
    s->sh4 = D_80194734 + 3;
    s->ot = D_8019D960.ot + otofs;
    verts = D_8019D960.verts;
    func_8006249C(r->p00, r->unk04);
    s->vtab = r->p00;
    s->nbase = r->p08;
    s->col = D_8008C990[lt];
    pk = (u16 *)r->prims;
    s->prim = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    do {
        s->hdr.w = *(u32 *)pk;
        pk += 2;
        switch (s->hdr.b.code & 0xFD) {
        case 0x24:
            a = VTX(pk[7]);
            b = VTX(pk[8]);
            c = VTX(pk[9]);
            func_800624E8(s, a->sxy, b->sxy, c->sxy);
            if (s->opz <= 0)
                break;
            func_8006250C(s->nbase + pk[6], &s->r);
            pf = (V62FT *)s->prim;
            s->prim = (u8 *)pf + 32;
            pf->xy0 = a->sxy;
            pf->xy1 = b->sxy;
            pf->xy2 = c->sxy;
            pf->uv0 = ((u32 *)pk)[0];
            pf->uv1 = ((u32 *)pk)[1];
            pf->uv2 = pk[4];
            setlen(pf, 7);
            gte_strgb(&pf->r0);
            z = (((a->sz + b->sz + c->sz) * 0x1555) >> s->sh3) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, pf);
            pf->code = (s->hdr.b.code & 2) | 0x24;
            break;
        case 0x2C:
            a = VTX(pk[9]);
            b = VTX(pk[10]);
            c = VTX(pk[11]);
            func_800624E8(s, a->sxy, b->sxy, c->sxy);
            if (s->opz <= 0)
                break;
            func_8006250C(s->nbase + pk[8], &s->r);
        ft4:
            pf = (V62FT *)s->prim;
            s->prim = (u8 *)pf + 40;
            d = VTX(pk[12]);
            pf->xy0 = a->sxy;
            pf->xy1 = b->sxy;
            pf->xy2 = c->sxy;
            pf->xy3 = d->sxy;
            pf->uv0 = ((u32 *)pk)[0];
            pf->uv1 = ((u32 *)pk)[1];
            pf->uv2 = pk[4];
            pf->uv3 = pk[6];
            setlen(pf, 9);
            gte_strgb(&pf->r0);
            z = ((a->sz + b->sz + c->sz + d->sz) >> s->sh4) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, pf);
            pf->code = (s->hdr.b.code & 2) | 0x2C;
            break;
        case 0x25:
            a = VTX(pk[7]);
            b = VTX(pk[8]);
            c = VTX(pk[9]);
            func_800624E8(s, a->sxy, b->sxy, c->sxy);
            if (s->opz <= 0)
                break;
            gte_ldv0(s->nbase + pk[6]);
            gte_ldrgb(s->col + IDX(pk[7]));
            gte_lddp(a->col);
            gte_ncds();
            pg = (V62GT *)s->prim;
            s->prim = (u8 *)pg + 40;
            gte_strgb(&pg->r0);
            gte_ldrgb(s->col + IDX(pk[8]));
            gte_lddp(b->col);
            gte_ncds();
            pg->xy0 = a->sxy;
            pg->xy1 = b->sxy;
            pg->xy2 = c->sxy;
            gte_strgb(pg->c1);
            gte_ldrgb(s->col + IDX(pk[9]));
            gte_lddp(c->col);
            gte_ncds();
            pg->uv0 = ((u32 *)pk)[0];
            pg->uv1 = ((u32 *)pk)[1];
            pg->uv2 = pk[4];
            setlen(pg, 9);
            pg->code = (s->hdr.b.code & 2) | 0x34;
            z = (((a->sz + b->sz + c->sz) * 0x1555) >> s->sh3) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, pg);
            gte_strgb(pg->c2);
            break;
        case 0x2D:
            a = VTX(pk[9]);
            b = VTX(pk[10]);
            c = VTX(pk[11]);
            func_800624E8(s, a->sxy, b->sxy, c->sxy);
            if (s->opz <= 0)
                break;
            gte_ldv0(s->nbase + pk[8]);
            gte_ldrgb(s->col + IDX(pk[9]));
            gte_lddp(a->col);
            gte_ncds();
            pg = (V62GT *)s->prim;
            s->prim = (u8 *)pg + 52;
            gte_strgb(&pg->r0);
            gte_ldrgb(s->col + IDX(pk[10]));
            gte_lddp(b->col);
            gte_ncds();
            d = VTX(pk[12]);
            pg->xy0 = a->sxy;
            pg->xy1 = b->sxy;
            pg->xy2 = c->sxy;
            pg->xy3 = d->sxy;
            gte_strgb(pg->c1);
            gte_ldrgb(s->col + IDX(pk[11]));
            gte_lddp(c->col);
            gte_ncds();
            pg->uv0 = ((u32 *)pk)[0];
            pg->uv1 = ((u32 *)pk)[1];
            pg->uv2 = pk[4];
            pg->uv3 = pk[6];
            gte_strgb(pg->c2);
            gte_ldrgb(s->col + IDX(pk[12]));
            gte_lddp(d->col);
            gte_ncds();
            setlen(pg, 12);
            pg->code = (s->hdr.b.code & 2) | 0x3C;
            z = ((a->sz + b->sz + c->sz + d->sz) >> s->sh4) + (s->hdr.b.b0 << 2);
            addPrim(s->ot + z, pg);
            gte_strgb(pg->c3);
            break;
        case 0x6C:
            func_8006250C(s->nbase + pk[8], &s->r);
            a = VTX(pk[9]);
            b = VTX(pk[10]);
            c = VTX(pk[11]);
            goto ft4;
        }
        pk += s->hdr.b.len * 2;
    } while (--s->count != 0);
    D_8019D95C.buf->cur = s->prim;
}
