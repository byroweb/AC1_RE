#include "ac1/ac1.h"

#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x10 projected vertex (D_801A6A48) */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    s16 pad06;
    u8 rgb[4];                  /* +0x08 */
    s32 dp;                     /* +0x0C */
} MVtx;

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
    MVtx vtx[1];                /* +0xB18 0x801A6A48 */
} MGlobals;

typedef struct {                /* gouraud triangle record (after the header word) */
    u32 uv0;
    u32 uv1;
    u16 uv2;
    u16 pad;
    u16 nrm;                    /* +0x0C byte offset of the face normal */
    u16 v0, v1, v2;             /* +0x0E byte offsets into vtx[] */
} TriRec;

typedef struct {                /* gouraud quad record */
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
    s8 zbias;                   /* +0x0C header word: OT bias */
    u8 size;                    /* +0x0D record size in words */
    u8 pad0e;
    u8 code;                    /* +0x0F */
    MPart *part;                /* +0x10 */
    u8 *normals;                /* +0x14 */
    void *verts;                /* +0x18 */
    s32 count;                  /* +0x1C */
    u32 *ot;                    /* +0x20 */
    s32 sh3;                    /* +0x24 */
    s32 sh4;                    /* +0x28 */
    u8 r, g, b;                 /* +0x2C */
} MScr;

#define MSCR ((MScr *)0x1F800000)

extern MGlobals D_801A5F30;
extern DrawCtx D_801A5F24;
extern s32 D_801987CC;
extern void *D_801A6544;
extern s32 D_801AD664;
extern void func_80064BE8(SVECTOR *v, s32 n);
extern void func_80063B94(SVECTOR *v, s32 n);
extern void func_80064628(MScr *s, s32 a, s32 b, s32 c);
extern void func_800645E8(u8 *nrm, u8 *rgb, s32 dp);
extern void func_8006460C(u8 *rgb, s32 dp);

/* draw one model part with gouraud lighting (NCDS per vertex) */
void func_80064684(u16 idx, s32 z)
{
    MScr *scr;
    MVtx *vb;
    u32 *rec;
    MVtx *a;
    MVtx *b;
    MVtx *c;
    MVtx *d;
    POLY_GT4 *q;
    s32 oz;

    scr = MSCR;
    MSCR->part = (MPart *)(D_801A5F30.model + (idx * 32 + 0xC) - idx * 4);
    if (MSCR->part->n_prims == 0) {
        return;
    }
    MSCR->count = MSCR->part->n_prims;
    MSCR->r = MSCR->g = MSCR->b = 64;
    MSCR->sh3 = D_801987CC + 15;
    MSCR->sh4 = D_801987CC + 3;
    MSCR->ot = &D_801A5F24.ot[z];
    vb = D_801A5F30.vtx;
    if (D_801AD664 == 0) {
        func_80064BE8(MSCR->part->verts, MSCR->part->n_verts);
    } else {
        func_80063B94(MSCR->part->verts, MSCR->part->n_verts);
    }
    scr->normals = scr->part->normals;
    scr->verts = D_801A6544;
    rec = scr->part->prims;
    scr->prim = D_801A5F24.buf->cur;
    scr->end = D_801A5F24.buf->end;
    do {
        *(u32 *)&scr->zbias = *rec++;
        switch (scr->code & 0xFC) {
        case 0x24:
            a = (MVtx *)((u8 *)vb + ((TriRec *)rec)->v0);
            b = (MVtx *)((u8 *)vb + ((TriRec *)rec)->v1);
            c = (MVtx *)((u8 *)vb + ((TriRec *)rec)->v2);
            func_80064628(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                func_800645E8(scr->normals + ((TriRec *)rec)->nrm, a->rgb, a->dp);
                q = (POLY_GT4 *)scr->prim;
                scr->prim = (u8 *)q + sizeof(POLY_GT3);
                gte_strgb(&q->r0);
                func_8006460C(b->rgb, b->dp);
                *(s32 *)&q->x0 = a->sxy;
                *(s32 *)&q->x1 = b->sxy;
                *(s32 *)&q->x2 = c->sxy;
                gte_strgb(&q->r1);
                func_8006460C(c->rgb, c->dp);
                *(u32 *)&q->u0 = ((TriRec *)rec)->uv0;
                *(u32 *)&q->u1 = ((TriRec *)rec)->uv1;
                *(u16 *)&q->u2 = ((TriRec *)rec)->uv2;
                setlen(q, 9);
                setcode(q, (scr->code & 2) | 0x34);
                oz = ((a->otz + b->otz + c->otz) * 0x1555 >> scr->sh3) + (scr->zbias << 2);
                addPrim(scr->ot + oz, q);
                gte_strgb(&q->r2);
            }
            break;
        case 0x2C:
            a = (MVtx *)((u8 *)vb + ((QuadRec *)rec)->v0);
            b = (MVtx *)((u8 *)vb + ((QuadRec *)rec)->v1);
            c = (MVtx *)((u8 *)vb + ((QuadRec *)rec)->v2);
            func_80064628(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                func_800645E8(scr->normals + ((QuadRec *)rec)->nrm, a->rgb, a->dp);
            quad:
                q = (POLY_GT4 *)scr->prim;
                d = (MVtx *)((u8 *)vb + ((QuadRec *)rec)->v3);
                scr->prim = (u8 *)(q + 1);
                gte_strgb(&q->r0);
                func_8006460C(b->rgb, b->dp);
                *(s32 *)&q->x0 = a->sxy;
                *(s32 *)&q->x1 = b->sxy;
                *(s32 *)&q->x2 = c->sxy;
                *(s32 *)&q->x3 = d->sxy;
                gte_strgb(&q->r1);
                func_8006460C(c->rgb, c->dp);
                *(u32 *)&q->u0 = ((QuadRec *)rec)->uv0;
                *(u32 *)&q->u1 = ((QuadRec *)rec)->uv1;
                *(u16 *)&q->u2 = ((QuadRec *)rec)->uv2;
                *(u16 *)&q->u3 = ((QuadRec *)rec)->uv3;
                gte_strgb(&q->r2);
                func_8006460C(d->rgb, d->dp);
                setlen(q, 12);
                setcode(q, (scr->code & 2) | 0x3C);
                oz = ((a->otz + b->otz + c->otz + d->otz) >> scr->sh4) + (scr->zbias << 2);
                addPrim(scr->ot + oz, q);
                gte_strgb(&q->r3);
            }
            break;
        case 0x6C:
            a = (MVtx *)((u8 *)vb + ((QuadRec *)rec)->v0);
            func_800645E8(scr->normals + ((QuadRec *)rec)->nrm, a->rgb, a->dp);
            b = (MVtx *)((u8 *)vb + ((QuadRec *)rec)->v1);
            c = (MVtx *)((u8 *)vb + ((QuadRec *)rec)->v2);
            goto quad;
        }
        rec += scr->size;
    } while (--scr->count);
    D_801A5F24.buf->cur = scr->prim;
}
