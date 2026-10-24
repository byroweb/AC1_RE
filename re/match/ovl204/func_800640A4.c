#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_ncdt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AF80416")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")
#define gte_strgb3(r0, r1, r2) __asm__ volatile ("swc2 $20, 0(%0)\n\tswc2 $21, 0(%1)\n\tswc2 $22, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")

typedef struct {                /* 0x10 projected vertex (0x801A6A48) */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    s16 pad06;
    u8 rgb[4];                  /* +0x08 */
    s32 dp;                     /* +0x0C */
} MVtx;

typedef struct {                /* 0x1C model part */
    SVECTOR *verts;             /* +0x00 */
    u16 n_verts;                /* +0x04 */
    u16 n_extra;                /* +0x06 extra (joint) vertices */
    u8 *normals;                /* +0x08 */
    u16 pad0c;
    u16 extra;                  /* +0x0E low 9 bits: extra primitives */
    u32 *prims;                 /* +0x10 */
    u16 n_prims;                /* +0x14 */
    u16 pad16;
    u32 pad18;
} MPart;

typedef struct {
    u32 *ot;                    /* +0x000 0x801A5F28 */
    u8 pad004[0x66C];
    MVtx vtx[1];                /* +0xB20 0x801A6A48 */
} OtVtx;

typedef struct {
    s32 nclip;                  /* +0x00 */
    u8 *prim;                   /* +0x04 */
    u8 *end;                    /* +0x08 */
    s8 zbias;                   /* +0x0C */
    u8 size;                    /* +0x0D */
    u8 pad0e;
    u8 code;                    /* +0x0F */
    MPart *part;                /* +0x10 */
    u8 *normals;                /* +0x14 */
    s32 count;                  /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 sh3;                    /* +0x20 */
    s32 sh4;                    /* +0x24 */
    s32 zmin;                   /* +0x28 */
} AScr;

#define ASCR ((AScr *)0x1F800000)

#define VTX(o) ((MVtx *)((u8 *)vb + (o)))
#define R16(o) (*(u16 *)((u8 *)rec + (o)))
#define R32(o) (*(u32 *)((u8 *)rec + (o)))
#define NRM(o) (scr->normals + R16(o))

extern OtVtx D_801A5F28;
extern u8 *D_801A5F30;
extern DrawCtx D_801A5F24;
extern s32 D_801987CC;
extern void func_800672CC(void);
extern void func_80064CB8(s32 n);
extern void func_80064D64(AScr *s, s32 a, s32 b, s32 c);

/* draw one part of an AC / object model lit by the scratchpad colour
 * (point lights, func_800672CC); bits adds the joint vertices / prims */
void func_800640A4(u16 idx, s32 z, s32 bits)
{
    AScr *scr;
    MVtx *vb;
    u32 *rec;
    MVtx *a;
    MVtx *b;
    MVtx *c;
    MVtx *d;
    POLY_GT4 *p;
    s32 oz;

    scr = ASCR;
    ASCR->zmin = 0;
    ASCR->sh3 = D_801987CC + 15;
    ASCR->sh4 = D_801987CC + 3;
    ASCR->ot = &D_801A5F28.ot[z];
    ASCR->part = (MPart *)(D_801A5F30 + (idx * 32 + 12) - idx * 4);
    vb = D_801A5F28.vtx;
    if (ASCR->part->n_verts == 0) {
        return;
    }
    func_800672CC();
    gte_ldrgb((u8 *)0x1F8000E0);
    if (bits) {
        func_80064CB8(ASCR->part->n_verts + ASCR->part->n_extra);
        ASCR->count = ASCR->part->n_prims + (ASCR->part->extra & 0x1FF);
    } else {
        func_80064CB8(ASCR->part->n_verts);
        ASCR->count = ASCR->part->n_prims;
    }
    scr->normals = scr->part->normals;
    rec = scr->part->prims;
    scr->count = scr->part->n_prims;
    if (bits) {
        scr->count += scr->part->extra & 0x1FF;
    }
    scr->prim = D_801A5F24.buf->cur;
    scr->end = D_801A5F24.buf->end;
    if (scr->end < scr->prim + (scr->count << 2) + (scr->count << 4) + (scr->count << 5)) {
        return;
    }
    while (scr->count--) {
        *(u32 *)&scr->zbias = *rec++;
        switch (scr->code & 0xFC) {
        case 0x24:
            a = VTX(R16(0xE));
            b = VTX(R16(0x10));
            c = VTX(R16(0x12));
            func_80064D64(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                gte_ldv0(NRM(0xC));
                gte_lddp(a->dp);
                gte_ncds();
                p = (POLY_GT4 *)scr->prim;
                scr->prim = (u8 *)p + sizeof(POLY_FT3);
                *(s32 *)&((POLY_FT3 *)p)->x0 = a->sxy;
                *(s32 *)&((POLY_FT3 *)p)->x1 = b->sxy;
                *(s32 *)&((POLY_FT3 *)p)->x2 = c->sxy;
                *(u32 *)&((POLY_FT3 *)p)->u0 = R32(0);
                *(u32 *)&((POLY_FT3 *)p)->u1 = R32(4);
                *(u16 *)&((POLY_FT3 *)p)->u2 = R16(8);
                setlen(p, 7);
                oz = (a->otz + b->otz + c->otz) * 0x1555 >> scr->sh3;
                if (scr->zmin < oz) {
                    addPrim(scr->ot + oz, p);
                    gte_strgb(&p->r0);
                    setcode(p, scr->code & 0xFE);
                }
            }
            break;
        case 0x2C:
            a = VTX(R16(0x12));
            b = VTX(R16(0x14));
            c = VTX(R16(0x16));
            func_80064D64(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                gte_ldv0(NRM(0x10));
                gte_lddp(a->dp);
                gte_ncds();
                p = (POLY_GT4 *)scr->prim;
                d = VTX(R16(0x18));
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
                oz = (a->otz + b->otz + c->otz + d->otz) >> scr->sh4;
                if (scr->zmin < oz) {
                    addPrim(scr->ot + oz, p);
                    gte_strgb(&p->r0);
                    setcode(p, scr->code & 0xFE);
                }
            }
            break;
        case 0x34:
            a = VTX(R16(0xE));
            b = VTX(R16(0x12));
            c = VTX(R16(0x16));
            func_80064D64(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                gte_ldv3(NRM(0xC), NRM(0x10), NRM(0x14));
                gte_lddp(a->dp);
                gte_ncdt();
                p = (POLY_GT4 *)scr->prim;
                scr->prim = (u8 *)p + sizeof(POLY_GT3);
                *(s32 *)&p->x0 = a->sxy;
                *(s32 *)&p->x1 = b->sxy;
                *(s32 *)&p->x2 = c->sxy;
                *(u32 *)&p->u0 = R32(0);
                *(u32 *)&p->u1 = R32(4);
                *(u16 *)&p->u2 = R16(8);
                setlen(p, 9);
                oz = (a->otz + b->otz + c->otz) * 0x1555 >> scr->sh3;
                if (scr->zmin < oz) {
                    addPrim(scr->ot + oz, p);
                    gte_strgb3(&p->r0, &p->r1, &p->r2);
                    setcode(p, scr->code & 0xFE);
                }
            }
            break;
        case 0x3C:
            a = VTX(R16(0x12));
            b = VTX(R16(0x16));
            c = VTX(R16(0x1A));
            func_80064D64(scr, a->sxy, b->sxy, c->sxy);
            if (scr->nclip > 0) {
                gte_ldv3(NRM(0x10), NRM(0x14), NRM(0x18));
                gte_lddp(a->dp);
                gte_ncdt();
                p = (POLY_GT4 *)scr->prim;
                d = VTX(R16(0x1E));
                scr->prim = (u8 *)(p + 1);
                *(s32 *)&p->x0 = a->sxy;
                *(s32 *)&p->x1 = b->sxy;
                *(s32 *)&p->x2 = c->sxy;
                *(s32 *)&p->x3 = d->sxy;
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(NRM(0x1C));
                gte_lddp(a->dp);
                gte_ncds();
                *(u32 *)&p->u0 = R32(0);
                *(u32 *)&p->u1 = R32(4);
                *(u16 *)&p->u2 = R16(8);
                *(u16 *)&p->u3 = R16(0xC);
                setlen(p, 12);
                setcode(p, scr->code & 0xFE);
                oz = (a->otz + b->otz + c->otz + d->otz) >> scr->sh4;
                if (scr->zmin < oz) {
                    addPrim(scr->ot + oz, p);
                    gte_strgb(&p->r3);
                }
            }
            break;
        case 0xA4:
            p = (POLY_GT4 *)scr->prim;
            a = VTX(R16(0x10));
            b = VTX(R16(0x12));
            c = VTX(R16(0x14));
            scr->prim = (u8 *)p + sizeof(POLY_FT3);
            *(s32 *)&((POLY_FT3 *)p)->x0 = a->sxy;
            *(s32 *)&((POLY_FT3 *)p)->x1 = b->sxy;
            *(s32 *)&((POLY_FT3 *)p)->x2 = c->sxy;
            *(u32 *)&p->r0 = R32(0xC);
            *(u32 *)&((POLY_FT3 *)p)->u0 = R32(0);
            *(u32 *)&((POLY_FT3 *)p)->u1 = R32(4);
            *(u16 *)&((POLY_FT3 *)p)->u2 = R16(8);
            setlen(p, 7);
            setcode(p, scr->code & 0x7F);
            oz = (a->otz + b->otz + c->otz) * 0x1555 >> scr->sh3;
            if (scr->zmin < oz) {
                addPrim(scr->ot + oz, p);
            }
            break;
        case 0xAC:
            p = (POLY_GT4 *)scr->prim;
            a = VTX(R16(0x14));
            b = VTX(R16(0x16));
            c = VTX(R16(0x18));
            d = VTX(R16(0x1A));
            scr->prim = (u8 *)p + sizeof(POLY_FT4);
            *(s32 *)&((POLY_FT4 *)p)->x0 = a->sxy;
            *(s32 *)&((POLY_FT4 *)p)->x1 = b->sxy;
            *(s32 *)&((POLY_FT4 *)p)->x2 = c->sxy;
            *(s32 *)&((POLY_FT4 *)p)->x3 = d->sxy;
            *(u32 *)&p->r0 = R32(0x10);
            *(u32 *)&((POLY_FT4 *)p)->u0 = R32(0);
            *(u32 *)&((POLY_FT4 *)p)->u1 = R32(4);
            *(u16 *)&((POLY_FT4 *)p)->u2 = R16(8);
            *(u16 *)&((POLY_FT4 *)p)->u3 = R16(0xC);
            setlen(p, 9);
            setcode(p, scr->code & 0x7F);
            oz = (a->otz + b->otz + c->otz + d->otz) >> scr->sh4;
            if (scr->zmin < oz) {
                addPrim(scr->ot + oz, p);
            }
            break;
        }
        rec += scr->size;
    }
    D_801A5F24.buf->cur = scr->prim;
}
