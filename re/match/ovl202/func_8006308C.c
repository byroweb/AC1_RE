#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x10 projected vertex (0x801A6A48) */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    s16 pad06;
    u8 rgb[4];                  /* +0x08 */
    s32 dp;                     /* +0x0C */
} MVtx;

typedef struct {                /* model part */
    SVECTOR *verts;             /* +0x00 */
    u16 n_verts;                /* +0x04 */
    u16 pad06;
    u8 *normals;                /* +0x08 */
    u32 pad0c;
    u32 *prims;                 /* +0x10 */
    u16 n_prims;                /* +0x14 */
} MPart;

typedef struct {
    u32 *ot;                    /* +0x000 0x801A5F28 */
    u8 pad004[0xB1C];
    MVtx vtx[1];                /* +0xB20 0x801A6A48 */
} OtVtx;

typedef struct {                /* triangle record (after the header word) */
    u32 uv0;
    u32 uv1;
    u16 uv2;
    u16 pad;
    u16 nrm;                    /* +0x0C */
    u16 v0, v1, v2;             /* +0x0E */
} TriRec;

typedef struct {                /* quad record */
    u32 uv0;
    u32 uv1;
    u16 uv2;
    u16 pad;
    u16 uv3;
    u16 pad2;
    u16 nrm;                    /* +0x10 */
    u16 v0, v1, v2, v3;         /* +0x12 */
} QuadRec;

typedef struct {
    s32 nclip;                  /* +0x00 */
    u8 *prim;                   /* +0x04 */
    u8 *end;                    /* +0x08 */
    s8 zbias;                   /* +0x0C */
    u8 size;                    /* +0x0D */
    u8 pad0e;
    u8 code;                    /* +0x0F */
    u8 *normals;                /* +0x10 */
    u8 *verts;                  /* +0x14 */
    s32 count;                  /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 sh3;                    /* +0x20 */
    s32 sh4;                    /* +0x24 */
    u8 *ctab;                   /* +0x28 */
    u8 r, g, b;                 /* +0x2C */
} WScr;

#define WSCR ((WScr *)0x1F800000)

#define VTX(o) ((MVtx *)((u8 *)vb + (o)))
#define PADCOL(o) (scr->ctab + ((SVECTOR *)(scr->verts + ((o) >> 1)))->pad)

extern OtVtx D_801A5F28;
extern DrawCtx D_801A5F24;
extern s32 D_801987CC;
extern u8 D_80090A28[][128];
extern void func_80062FFC(SVECTOR *v, s32 n);
extern void func_80063048(WScr *s, s32 a, s32 b, s32 c);
extern void func_8006306C(u8 *nrm, u8 *rgb);

/* draw one world-structure model part: flat (NCCS) or per-vertex coloured
 * (colour table D_80090A28 by the vertex pad) textured polygons */
void func_8006308C(MPart *part, s32 nib, s32 z)
{
    WScr *scr;
    MVtx *vb;
    u32 *rec;
    MVtx *a;
    MVtx *b;
    MVtx *c;
    MVtx *d;
    POLY_GT4 *p;
    s32 oz;

    scr = WSCR;
    WSCR->count = part->n_prims;
    if (part->n_prims == 0) {
        return;
    }
    WSCR->r = WSCR->g = WSCR->b = 64;
    WSCR->sh3 = D_801987CC + 15;
    WSCR->sh4 = D_801987CC + 3;
    WSCR->ot = &D_801A5F28.ot[z];
    vb = D_801A5F28.vtx;
    func_80062FFC(part->verts, part->n_verts);
    WSCR->verts = (u8 *)part->verts;
    WSCR->normals = part->normals;
    WSCR->ctab = D_80090A28[nib];
    rec = part->prims;
    WSCR->prim = D_801A5F24.buf->cur;
    WSCR->end = D_801A5F24.buf->end;
    do {
        *(u32 *)&scr->zbias = *rec++;
        switch (scr->code & 0xFD) {
        case 0x24:
            a = VTX(((TriRec *)rec)->v0);
            b = VTX(((TriRec *)rec)->v1);
            c = VTX(((TriRec *)rec)->v2);
            func_80063048(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                func_8006306C(scr->normals + ((TriRec *)rec)->nrm, &scr->r);
                p = (POLY_GT4 *)scr->prim;
                scr->prim = (u8 *)p + sizeof(POLY_FT3);
                *(s32 *)&((POLY_FT3 *)p)->x0 = a->sxy;
                *(s32 *)&((POLY_FT3 *)p)->x1 = b->sxy;
                *(s32 *)&((POLY_FT3 *)p)->x2 = c->sxy;
                *(u32 *)&((POLY_FT3 *)p)->u0 = ((TriRec *)rec)->uv0;
                *(u32 *)&((POLY_FT3 *)p)->u1 = ((TriRec *)rec)->uv1;
                *(u16 *)&((POLY_FT3 *)p)->u2 = ((TriRec *)rec)->uv2;
                setlen(p, 7);
                gte_strgb(&p->r0);
                oz = ((a->otz + b->otz + c->otz) * 0x1555 >> scr->sh3) + (scr->zbias << 2);
                addPrim(scr->ot + oz, p);
                setcode(p, (scr->code & 2) | 0x24);
            }
            break;
        case 0x2C:
            a = VTX(((QuadRec *)rec)->v0);
            b = VTX(((QuadRec *)rec)->v1);
            c = VTX(((QuadRec *)rec)->v2);
            func_80063048(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                func_8006306C(scr->normals + ((QuadRec *)rec)->nrm, &scr->r);
            fquad:
                p = (POLY_GT4 *)scr->prim;
                scr->prim = (u8 *)p + sizeof(POLY_FT4);
                d = VTX(((QuadRec *)rec)->v3);
                *(s32 *)&((POLY_FT4 *)p)->x0 = a->sxy;
                *(s32 *)&((POLY_FT4 *)p)->x1 = b->sxy;
                *(s32 *)&((POLY_FT4 *)p)->x2 = c->sxy;
                *(s32 *)&((POLY_FT4 *)p)->x3 = d->sxy;
                *(u32 *)&((POLY_FT4 *)p)->u0 = ((QuadRec *)rec)->uv0;
                *(u32 *)&((POLY_FT4 *)p)->u1 = ((QuadRec *)rec)->uv1;
                *(u16 *)&((POLY_FT4 *)p)->u2 = ((QuadRec *)rec)->uv2;
                *(u16 *)&((POLY_FT4 *)p)->u3 = ((QuadRec *)rec)->uv3;
                setlen(p, 9);
                gte_strgb(&p->r0);
                oz = ((a->otz + b->otz + c->otz + d->otz) >> scr->sh4) + (scr->zbias << 2);
                addPrim(scr->ot + oz, p);
                setcode(p, (scr->code & 2) | 0x2C);
            }
            break;
        case 0x25:
            a = VTX(((TriRec *)rec)->v0);
            b = VTX(((TriRec *)rec)->v1);
            c = VTX(((TriRec *)rec)->v2);
            func_80063048(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                gte_ldv0(scr->normals + ((TriRec *)rec)->nrm);
                gte_ldrgb(PADCOL(((TriRec *)rec)->v0));
                gte_lddp(a->dp);
                gte_ncds();
                p = (POLY_GT4 *)scr->prim;
                scr->prim = (u8 *)p + sizeof(POLY_GT3);
                gte_strgb(&p->r0);
                gte_ldrgb(PADCOL(((TriRec *)rec)->v1));
                gte_lddp(b->dp);
                gte_ncds();
                *(s32 *)&p->x0 = a->sxy;
                *(s32 *)&p->x1 = b->sxy;
                *(s32 *)&p->x2 = c->sxy;
                gte_strgb(&p->r1);
                gte_ldrgb(PADCOL(((TriRec *)rec)->v2));
                gte_lddp(c->dp);
                gte_ncds();
                *(u32 *)&p->u0 = ((TriRec *)rec)->uv0;
                *(u32 *)&p->u1 = ((TriRec *)rec)->uv1;
                *(u16 *)&p->u2 = ((TriRec *)rec)->uv2;
                setlen(p, 9);
                setcode(p, (scr->code & 2) | 0x34);
                oz = ((a->otz + b->otz + c->otz) * 0x1555 >> scr->sh3) + (scr->zbias << 2);
                addPrim(scr->ot + oz, p);
                gte_strgb(&p->r2);
            }
            break;
        case 0x2D:
            a = VTX(((QuadRec *)rec)->v0);
            b = VTX(((QuadRec *)rec)->v1);
            c = VTX(((QuadRec *)rec)->v2);
            func_80063048(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                gte_ldv0(scr->normals + ((QuadRec *)rec)->nrm);
                gte_ldrgb(PADCOL(((QuadRec *)rec)->v0));
                gte_lddp(a->dp);
                gte_ncds();
                p = (POLY_GT4 *)scr->prim;
                scr->prim = (u8 *)(p + 1);
                gte_strgb(&p->r0);
                gte_ldrgb(PADCOL(((QuadRec *)rec)->v1));
                gte_lddp(b->dp);
                gte_ncds();
                d = VTX(((QuadRec *)rec)->v3);
                *(s32 *)&p->x0 = a->sxy;
                *(s32 *)&p->x1 = b->sxy;
                *(s32 *)&p->x2 = c->sxy;
                *(s32 *)&p->x3 = d->sxy;
                gte_strgb(&p->r1);
                gte_ldrgb(PADCOL(((QuadRec *)rec)->v2));
                gte_lddp(c->dp);
                gte_ncds();
                *(u32 *)&p->u0 = ((QuadRec *)rec)->uv0;
                *(u32 *)&p->u1 = ((QuadRec *)rec)->uv1;
                *(u16 *)&p->u2 = ((QuadRec *)rec)->uv2;
                *(u16 *)&p->u3 = ((QuadRec *)rec)->uv3;
                gte_strgb(&p->r2);
                gte_ldrgb(PADCOL(((QuadRec *)rec)->v3));
                gte_lddp(d->dp);
                gte_ncds();
                setlen(p, 12);
                setcode(p, (scr->code & 2) | 0x3C);
                oz = ((a->otz + b->otz + c->otz + d->otz) >> scr->sh4) + (scr->zbias << 2);
                addPrim(scr->ot + oz, p);
                gte_strgb(&p->r3);
            }
            break;
        case 0x6C:
            func_8006306C(scr->normals + ((QuadRec *)rec)->nrm, &scr->r);
            a = VTX(((QuadRec *)rec)->v0);
            b = VTX(((QuadRec *)rec)->v1);
            c = VTX(((QuadRec *)rec)->v2);
            goto fquad;
        }
        rec += scr->size;
    } while (--scr->count);
    D_801A5F24.buf->cur = scr->prim;
}
