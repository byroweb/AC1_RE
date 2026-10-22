#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x10 projected vertex with clip codes (0x801A6A48) */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    u8 clip;                    /* +0x06 */
    u8 near;                    /* +0x07 */
    u8 col[3];                  /* +0x08 */
    u8 lit;                     /* +0x0B */
    s32 dp;                     /* +0x0C */
} CVtx;

typedef struct {                /* 0x1C model part */
    SVECTOR *verts;             /* +0x00 */
    u16 n_verts;                /* +0x04 */
    u16 pad06;
    u8 *normals;                /* +0x08 */
    u32 pad0c;
    u32 *prims;                 /* +0x10 */
    u16 n_prims;                /* +0x14 */
    u16 pad16;
    u32 pad18;
} MPart;

typedef struct {
    u8 *model;                  /* +0x000 0x801A5F30 */
    u8 pad004[0xB14];
    CVtx vtx[1];                /* +0xB18 0x801A6A48 */
} MGlobals;

typedef struct {                /* deferred (near) primitive */
    u32 *rec;
    u32 hdr;
} PList;

typedef struct {
    s32 nclip;                  /* +0x000 */
    u8 *prim;                   /* +0x004 */
    u8 *end;                    /* +0x008 */
    u8 pad00c[0x344];
    s8 zbias;                   /* +0x350 header word */
    u8 size;                    /* +0x351 */
    u8 sub;                     /* +0x352 subdivision levels: near 1 low 3 bits, near 2 high */
    u8 code;                    /* +0x353 */
    MPart *part;                /* +0x354 */
    u8 *normals;                /* +0x358 */
    SVECTOR *verts;             /* +0x35C */
    u8 pad360[8];
    s32 count;                  /* +0x368 */
    u32 *ot;                    /* +0x36C */
    u8 pad370[8];
    s32 sh3;                    /* +0x378 */
    s32 sh4;                    /* +0x37C */
    PList *l0;                  /* +0x380 quads, flat */
    PList *l1;                  /* +0x384 triangles, flat */
    PList *l2;                  /* +0x388 quads, lit */
    PList *l3;                  /* +0x38C triangles, lit */
    u8 r, g, b;                 /* +0x390 */
} CScr;

#define CSCR ((CScr *)0x1F800000)

#define VTX(o) ((CVtx *)((u8 *)vb + (o)))
#define R16(o) (*(u16 *)((u8 *)rec + (o)))
#define R32(o) (*(u32 *)((u8 *)rec + (o)))
#define NRM(o) (scr->normals + R16(o))

extern MGlobals D_801A5F30;
extern DrawCtx D_801A5F24;
extern SVECTOR *D_801A6544;
extern s32 D_801987C8;
extern void func_80067BE0(s32 n);
extern s32 func_80067E54(s32 a, s32 b, s32 c);
extern void func_80067E84(u8 *nrm, u8 *col);
extern void func_80067EA4(u8 *col);
extern void func_80069F04(PList *l);
extern void func_8006A7EC(PList *l);
extern void func_8006C580(PList *l);
extern void func_8006D0C0(PList *l);

/* draw a model part with near-plane handling: primitives near the camera
 * go to four lists that the subdividers draw afterwards */
void func_80067EBC(u16 idx)
{
    PList b0[256];
    PList b1[256];
    PList b2[256];
    PList b3[256];
    CScr *scr;
    CVtx *vb;
    u32 *rec;
    CVtx *a;
    CVtx *b;
    CVtx *c;
    CVtx *d;
    POLY_GT4 *p;
    u32 n;
    s32 oz;
    s32 s;

    scr = CSCR;
    CSCR->part = (MPart *)(D_801A5F30.model + (idx * 32 + 12) - idx * 4);
    if (CSCR->part->n_prims == 0) {
        return;
    }
    CSCR->count = CSCR->part->n_prims;
    CSCR->r = CSCR->g = CSCR->b = 64;
    CSCR->l0 = b0;
    CSCR->l1 = b1;
    CSCR->l2 = b2;
    CSCR->l3 = b3;
    CSCR->sh3 = D_801987C8 + 15;
    CSCR->sh4 = D_801987C8 + 3;
    CSCR->ot = D_801A5F24.ot + 120;
    vb = D_801A5F30.vtx;
    func_80067BE0(CSCR->part->n_verts);
    CSCR->normals = CSCR->part->normals;
    CSCR->verts = D_801A6544;
    rec = CSCR->part->prims;
    CSCR->prim = D_801A5F24.buf->cur;
    CSCR->end = D_801A5F24.buf->end;
    do {
        *(u32 *)&scr->zbias = *rec++;
        switch (scr->code & 0xFC) {
        case 0x24:
            a = VTX(R16(0xE));
            b = VTX(R16(0x10));
            c = VTX(R16(0x12));
            if (c->clip & (a->clip & b->clip)) {
                break;
            }
            n = c->near | (a->near | b->near);
            if (n == 0) {
            tri:
                if ((c->clip | (a->clip | b->clip)) == 31) {
                    break;
                }
                if (func_80067E54(a->sxy, b->sxy, c->sxy) <= 0) {
                    break;
                }
                if ((c->lit | (a->lit | b->lit)) == 0) {
                    gte_ldv0(NRM(0xC));
                    gte_ldrgb(&scr->r);
                    gte_nccs();
                    p = (POLY_GT4 *)scr->prim;
                    scr->prim = (u8 *)p + sizeof(POLY_FT3);
                    *(s32 *)&((POLY_FT3 *)p)->x0 = a->sxy;
                    *(s32 *)&((POLY_FT3 *)p)->x1 = b->sxy;
                    *(s32 *)&((POLY_FT3 *)p)->x2 = c->sxy;
                    *(u32 *)&((POLY_FT3 *)p)->u0 = R32(0);
                    *(u32 *)&((POLY_FT3 *)p)->u1 = R32(4);
                    *(u16 *)&((POLY_FT3 *)p)->u2 = R16(8);
                    setlen(p, 7);
                    gte_strgb(&p->r0);
                    setcode(p, scr->code);
                    oz = ((a->otz + b->otz + c->otz) * 0x1555 >> scr->sh3) + (scr->zbias << 2);
                    addPrim(scr->ot + oz, p);
                    break;
                } else {
                    func_80067E84(NRM(0xC), a->col);
                    p = (POLY_GT4 *)scr->prim;
                    scr->prim = (u8 *)p + sizeof(POLY_GT3);
                    gte_strgb(&p->r0);
                    func_80067EA4(b->col);
                    *(s32 *)&p->x0 = a->sxy;
                    *(s32 *)&p->x1 = b->sxy;
                    *(s32 *)&p->x2 = c->sxy;
                    gte_strgb(&p->r1);
                    func_80067EA4(c->col);
                    *(u32 *)&p->u0 = R32(0);
                    *(u32 *)&p->u1 = R32(4);
                    *(u16 *)&p->u2 = R16(8);
                    setlen(p, 9);
                    setcode(p, (scr->code & 2) | 0x34);
                    gte_strgb(&p->r2);
                    oz = ((a->otz + b->otz + c->otz) * 0x1555 >> scr->sh3) + (scr->zbias << 2);
                    addPrim(scr->ot + oz, p);
                    break;
                }
            }
            if (n < 2) {
                s = scr->sub & 7;
                if (s == 0) {
                    goto tri;
                }
                if (func_80067E54(a->sxy, b->sxy, c->sxy) <= 0) {
                    break;
                }
            } else {
                s = scr->sub >> 3;
                if (s == 0) {
                    goto tri;
                }
            }
            if ((c->lit | (a->lit | b->lit)) == 0) {
                scr->l1->rec = rec;
                scr->sub = s;
                scr->l1->hdr = *(u32 *)&scr->zbias;
                scr->l1++;
            } else {
                scr->l3->rec = rec;
                scr->sub = s;
                scr->l3->hdr = *(u32 *)&scr->zbias;
                scr->l3++;
            }
            break;
        case 0x2C:
            a = VTX(R16(0x12));
            b = VTX(R16(0x14));
            c = VTX(R16(0x16));
            d = VTX(R16(0x18));
            if (d->clip & (c->clip & (a->clip & b->clip))) {
                break;
            }
            n = d->near | (c->near | (a->near | b->near));
            if (n == 0) {
            quad:
                if ((d->clip | (c->clip | (a->clip | b->clip))) == 31) {
                    break;
                }
                if (func_80067E54(a->sxy, b->sxy, c->sxy) <= 0) {
                    break;
                }
                if ((d->lit | (c->lit | (a->lit | b->lit))) == 0) {
                    gte_ldv0(NRM(0x10));
                    gte_ldrgb(&scr->r);
                    gte_nccs();
                    p = (POLY_GT4 *)scr->prim;
                    scr->prim = (u8 *)p + sizeof(POLY_FT4);
                    *(s32 *)&((POLY_FT4 *)p)->x0 = a->sxy;
                    *(s32 *)&((POLY_FT4 *)p)->x1 = b->sxy;
                    *(s32 *)&((POLY_FT4 *)p)->x2 = c->sxy;
                    *(s32 *)&((POLY_FT4 *)p)->x3 = d->sxy;
                    *(u32 *)&((POLY_FT4 *)p)->u0 = R32(0);
                    *(u32 *)&((POLY_FT4 *)p)->u1 = R32(4);
                    *(u16 *)&((POLY_FT4 *)p)->u2 = R16(8);
                    *(u16 *)&((POLY_FT4 *)p)->u3 = R16(0xC);
                    setlen(p, 9);
                    gte_strgb(&p->r0);
                    setcode(p, scr->code);
                    oz = ((a->otz + b->otz + c->otz + d->otz) >> scr->sh4) + (scr->zbias << 2);
                    addPrim(scr->ot + oz, p);
                    break;
                } else {
                gquad:
                    func_80067E84(NRM(0x10), a->col);
                    p = (POLY_GT4 *)scr->prim;
                    scr->prim = (u8 *)(p + 1);
                    gte_strgb(&p->r0);
                    func_80067EA4(b->col);
                    *(s32 *)&p->x0 = a->sxy;
                    *(s32 *)&p->x1 = b->sxy;
                    *(s32 *)&p->x2 = c->sxy;
                    *(s32 *)&p->x3 = d->sxy;
                    gte_strgb(&p->r1);
                    func_80067EA4(c->col);
                    *(u32 *)&p->u0 = R32(0);
                    *(u32 *)&p->u1 = R32(4);
                    *(u16 *)&p->u2 = R16(8);
                    *(u16 *)&p->u3 = R16(0xC);
                    gte_strgb(&p->r2);
                    func_80067EA4(d->col);
                    setlen(p, 12);
                    setcode(p, (scr->code & 2) | 0x3C);
                    gte_strgb(&p->r3);
                    oz = ((a->otz + b->otz + c->otz + d->otz) >> scr->sh4) + (scr->zbias << 2);
                    addPrim(scr->ot + oz, p);
                    break;
                }
            }
            if (n < 2) {
                s = scr->sub & 7;
                if (s == 0) {
                    goto quad;
                }
                if (func_80067E54(a->sxy, b->sxy, c->sxy) <= 0) {
                    break;
                }
            } else {
                s = scr->sub >> 3;
                if (s == 0) {
                    goto quad;
                }
            }
            if ((d->lit | (c->lit | (a->lit | b->lit))) == 0) {
                goto ql0;
            }
            goto ql2;
        case 0x6C:
            a = VTX(R16(0x12));
            b = VTX(R16(0x14));
            c = VTX(R16(0x16));
            d = VTX(R16(0x18));
            if (d->clip & (c->clip & (a->clip & b->clip))) {
                break;
            }
            n = d->near | (c->near | (a->near | b->near));
            if (n == 0) {
            quad2:
                if ((d->clip | (c->clip | (a->clip | b->clip))) == 31) {
                    break;
                }
                if ((d->lit | (c->lit | (a->lit | b->lit))) != 0) {
                    goto gquad;
                }
                gte_ldv0(NRM(0x10));
                gte_ldrgb(&scr->r);
                gte_nccs();
                p = (POLY_GT4 *)scr->prim;
                scr->prim = (u8 *)p + sizeof(POLY_FT4);
                *(s32 *)&((POLY_FT4 *)p)->x0 = a->sxy;
                *(s32 *)&((POLY_FT4 *)p)->x1 = b->sxy;
                *(s32 *)&((POLY_FT4 *)p)->x2 = c->sxy;
                *(s32 *)&((POLY_FT4 *)p)->x3 = d->sxy;
                *(u32 *)&((POLY_FT4 *)p)->u0 = R32(0);
                *(u32 *)&((POLY_FT4 *)p)->u1 = R32(4);
                *(u16 *)&((POLY_FT4 *)p)->u2 = R16(8);
                *(u16 *)&((POLY_FT4 *)p)->u3 = R16(0xC);
                setlen(p, 9);
                gte_strgb(&p->r0);
                setcode(p, scr->code & 0xBF);
                oz = ((a->otz + b->otz + c->otz + d->otz) >> scr->sh4) + (scr->zbias << 2);
                addPrim(scr->ot + oz, p);
                break;
            }
            if (n < 2) {
                s = scr->sub & 7;
            } else {
                s = scr->sub >> 3;
            }
            if (s == 0) {
                goto quad2;
            }
            if ((d->lit | (c->lit | (a->lit | b->lit))) == 0) {
            ql0:
                scr->l0->rec = rec;
                scr->sub = s;
                scr->l0->hdr = *(u32 *)&scr->zbias;
                scr->l0++;
            } else {
            ql2:
                scr->l2->rec = rec;
                scr->sub = s;
                scr->l2->hdr = *(u32 *)&scr->zbias;
                scr->l2++;
            }
            break;
        }
        rec += scr->size;
    } while (--scr->count);
    scr->sh3 -= 14;
    scr->l1->rec = 0;
    func_80069F04(b1);
    scr->l0->rec = 0;
    func_8006A7EC(b0);
    scr->l3->rec = 0;
    func_8006C580(b3);
    scr->l2->rec = 0;
    func_8006D0C0(b2);
    D_801A5F24.buf->cur = scr->prim;
}
