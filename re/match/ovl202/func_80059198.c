#include "ac1/ac1.h"
/* ovl202 0x80059198 - draw part `part` of the current model with depth-cued GTE
 * lighting: vertices are lit and coloured by func_80058E04 first; flat-normal polygons
 * 0x25/0x2D become gouraud (NCS, then a depth cue per vertex colour), 0x35/0x3D
 * NCDS per vertex, 0x24/0x2C flat lit, 0x34/0x3C gouraud lit, 0xA4/0xAC unlit. */

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ncs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AC8041E")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")
#define gte_strgb3(r0, r1, r2) __asm__ volatile ( \
    "swc2 $20, 0(%0)\n\tswc2 $21, 0(%1)\n\tswc2 $22, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")

typedef struct {                /* 0x10 transformed vertex (0x801A6A48) */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    u8 pad06[2];
    u32 col;                    /* +0x08 */
    s32 dp;                     /* +0x0C */
} LVtx;

typedef struct {                /* 0x1C model record */
    SVECTOR *verts;             /* +0x00 */
    u16 nv0;                    /* +0x04 */
    u16 nv1;                    /* +0x06 */
    u8 *nrm;                    /* +0x08 */
    u8 pad0c[2];
    u16 flags;                  /* +0x0E low 9 bits: extra packets */
    u32 *pkt;                   /* +0x10 */
    u16 npkt;                   /* +0x14 */
} PartRec;

typedef struct {
    u32 tag;
    u32 rgbc;
    s32 xy0;
    u32 uv0;
    s32 xy1;
    u32 uv1;
    s32 xy2;
    u16 uv2;
    u16 pad;
} PFT3;

typedef struct {
    u32 tag;
    u32 rgbc;
    s32 xy0;
    u32 uv0;
    s32 xy1;
    u32 uv1;
    s32 xy2;
    u16 uv2;
    u16 pad2;
    s32 xy3;
    u16 uv3;
    u16 pad3;
} PFT4;

typedef struct {
    u32 tag;
    u32 rgbc0;
    s32 xy0;
    u32 uv0;
    u32 rgb1;
    s32 xy1;
    u32 uv1;
    u32 rgb2;
    s32 xy2;
    u16 uv2;
    u16 pad2;
} PGT3;

typedef struct {
    u32 tag;
    u32 rgbc0;
    s32 xy0;
    u32 uv0;
    u32 rgb1;
    s32 xy1;
    u32 uv1;
    u32 rgb2;
    s32 xy2;
    u16 uv2;
    u16 pad2;
    u32 rgb3;
    s32 xy3;
    u16 uv3;
    u16 pad3;
} PGT4;

typedef struct {
    s32 opz;                    /* +0x00 */
    u8 *prim;                   /* +0x04 */
    u8 *end;                    /* +0x08 */
    union {
        u32 w;
        struct { u8 b0; u8 len; u8 b2; u8 code; } b;
    } hdr;                      /* +0x0C */
    PartRec *rec;               /* +0x10 */
    u8 *nrm;                    /* +0x14 */
    s32 n;                      /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 shift4;                 /* +0x20 */
    s32 shift;                  /* +0x24 */
    s32 zmin;                   /* +0x28 */
    u8 col[4];                  /* +0x2C */
    SVECTOR sv;                 /* +0x30 */
} LSpad;

typedef struct {                /* 0x801A5F24 */
    DrawCtx ctx;
    u8 pad08[4];
    u8 *model;                  /* +0x0C (0x801A5F30) */
    u8 pad10[0xB24 - 0x10];
    LVtx vtx[1];                /* +0xB24 = 0x801A6A48 */
} DrawWork;

extern DrawWork D_801A5F24;
extern s32 D_801987CC;
extern void func_80058E04(void *o, u16 mask, s32 extra, u32 **ot);
extern void func_800590C4(s32 *opz, s32 a, s32 b, s32 c);
extern void func_800590E8(SVECTOR *ir, void *rgb, s32 p);
extern void func_8005911C(void *v, void *rgb, s32 p);
extern void func_80059140(void *v0, void *v1, void *v2, void *rgb, s32 p);
extern void func_80059178(SVECTOR *out);

#define VTX(o) ((LVtx *)((u8 *)vtx + (o)))

void func_80059198(u16 part, s32 z, u16 mask, s32 extra)
{
    LSpad *sp;
    PartRec *rec;
    u16 *pk;
    LVtx *v0;
    LVtx *v1;
    LVtx *v2;
    LVtx *v3;
    u8 *p;
    u8 *nrm;
    LVtx *vtx;
    s32 oz;
    s32 n;

    sp = (LSpad *)0x1F800000;
    sp->col[0] = sp->col[1] = sp->col[2] = 64;
    sp->zmin = 0;
    sp->shift4 = D_801987CC + 15;
    sp->shift = D_801987CC + 3;
    sp->ot = D_801A5F24.ctx.ot + z;
    rec = (PartRec *)(D_801A5F24.model + (part * 32 + 12) - part * 4);
    sp->rec = rec;
    vtx = D_801A5F24.vtx;
    if (rec->nv0 == 0) {
        return;
    }
    func_80058E04(sp, mask, extra, &D_801A5F24.ctx.ot);
    sp->nrm = sp->rec->nrm;
    pk = (u16 *)sp->rec->pkt;
    if (extra) {
        sp->n = sp->rec->npkt + (sp->rec->flags & 0x1FF);
    } else {
        sp->n = sp->rec->npkt;
    }
    n = sp->n;
    sp->prim = D_801A5F24.ctx.buf->cur;
    sp->end = D_801A5F24.ctx.buf->end;
    if ((u32)sp->end < (u32)(sp->prim + n * 4 + n * 16 + n * 32)) {
        return;
    }
    sp->n = n - 1;
    if (n != 0) {
        do {
            sp->hdr.w = *(u32 *)pk;
            pk += 2;
            switch (sp->hdr.b.code & 0xFD) {
            case 0x25:
                v0 = VTX(pk[7]);
                v1 = VTX(pk[8]);
                v2 = VTX(pk[9]);
                func_800590C4(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                gte_ldv0(sp->nrm + pk[6]);
                gte_ncs();
                p = sp->prim;
                sp->prim = p + sizeof(PGT3);
                ((PGT3 *)p)->xy0 = v0->sxy;
                ((PGT3 *)p)->xy1 = v1->sxy;
                ((PGT3 *)p)->xy2 = v2->sxy;
                func_80059178(&sp->sv);
                func_800590E8(&sp->sv, &v0->col, v0->dp);
                gte_strgb(&((PGT3 *)p)->rgbc0);
                func_800590E8(&sp->sv, &v1->col, v1->dp);
                ((PGT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PGT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PGT3 *)p)->uv2 = pk[4];
                gte_strgb(&((PGT3 *)p)->rgb1);
                func_800590E8(&sp->sv, &v2->col, v2->dp);
                setlen(p, 9);
                ((P_TAG *)p)->code = (sp->hdr.b.code & 2) | 0x34;
                oz = ((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                    gte_strgb(&((PGT3 *)p)->rgb2);
                }
                break;
            case 0x2D:
                v0 = VTX(pk[9]);
                v1 = VTX(pk[10]);
                v2 = VTX(pk[11]);
                func_800590C4(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                gte_ldv0(sp->nrm + pk[8]);
                gte_ncs();
                p = sp->prim;
                v3 = VTX(pk[12]);
                sp->prim = p + sizeof(PGT4);
                ((PGT4 *)p)->xy0 = v0->sxy;
                ((PGT4 *)p)->xy1 = v1->sxy;
                ((PGT4 *)p)->xy2 = v2->sxy;
                ((PGT4 *)p)->xy3 = v3->sxy;
                func_80059178(&sp->sv);
                func_800590E8(&sp->sv, &v0->col, v0->dp);
                gte_strgb(&((PGT4 *)p)->rgbc0);
                func_800590E8(&sp->sv, &v1->col, v1->dp);
                gte_strgb(&((PGT4 *)p)->rgb1);
                func_800590E8(&sp->sv, &v2->col, v2->dp);
                ((PGT4 *)p)->uv0 = ((u32 *)pk)[0];
                ((PGT4 *)p)->uv1 = ((u32 *)pk)[1];
                ((PGT4 *)p)->uv2 = pk[4];
                ((PGT4 *)p)->uv3 = pk[6];
                gte_strgb(&((PGT4 *)p)->rgb2);
                func_800590E8(&sp->sv, &v3->col, v3->dp);
                setlen(p, 12);
                ((P_TAG *)p)->code = (sp->hdr.b.code & 2) | 0x3C;
                oz = (v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                    gte_strgb(&((PGT4 *)p)->rgb3);
                }
                break;
            case 0x35:
                v0 = VTX(pk[7]);
                v1 = VTX(pk[9]);
                v2 = VTX(pk[11]);
                func_800590C4(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                func_8005911C(sp->nrm + pk[6], &v0->col, v0->dp);
                p = sp->prim;
                sp->prim = p + sizeof(PGT3);
                ((PGT3 *)p)->xy0 = v0->sxy;
                ((PGT3 *)p)->xy1 = v1->sxy;
                ((PGT3 *)p)->xy2 = v2->sxy;
                gte_strgb(&((PGT3 *)p)->rgbc0);
                func_8005911C(sp->nrm + pk[8], &v1->col, v1->dp);
                gte_strgb(&((PGT3 *)p)->rgb1);
                func_8005911C(sp->nrm + pk[10], &v2->col, v2->dp);
                ((PGT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PGT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PGT3 *)p)->uv2 = pk[4];
                setlen(p, 9);
                ((P_TAG *)p)->code = sp->hdr.b.code & 0xFE;
                oz = ((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                    gte_strgb(&((PGT3 *)p)->rgb2);
                }
                break;
            case 0x3D:
                v0 = VTX(pk[9]);
                v1 = VTX(pk[11]);
                v2 = VTX(pk[13]);
                func_800590C4(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                func_8005911C(sp->nrm + pk[8], &v0->col, v0->dp);
                p = sp->prim;
                v3 = VTX(pk[15]);
                sp->prim = p + sizeof(PGT4);
                gte_strgb(&((PGT4 *)p)->rgbc0);
                func_8005911C(sp->nrm + pk[10], &v1->col, v1->dp);
                ((PGT4 *)p)->xy0 = v0->sxy;
                ((PGT4 *)p)->xy1 = v1->sxy;
                ((PGT4 *)p)->xy2 = v2->sxy;
                ((PGT4 *)p)->xy3 = v3->sxy;
                gte_strgb(&((PGT4 *)p)->rgb1);
                func_8005911C(sp->nrm + pk[12], &v2->col, v2->dp);
                gte_strgb(&((PGT4 *)p)->rgb2);
                func_8005911C(sp->nrm + pk[14], &v3->col, v3->dp);
                ((PGT4 *)p)->uv0 = ((u32 *)pk)[0];
                ((PGT4 *)p)->uv1 = ((u32 *)pk)[1];
                ((PGT4 *)p)->uv2 = pk[4];
                ((PGT4 *)p)->uv3 = pk[6];
                setlen(p, 12);
                ((P_TAG *)p)->code = sp->hdr.b.code & 0xFE;
                oz = (v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                    gte_strgb(&((PGT4 *)p)->rgb3);
                }
                break;
            case 0x24:
                v0 = VTX(pk[7]);
                v1 = VTX(pk[8]);
                v2 = VTX(pk[9]);
                func_800590C4(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                func_8005911C(sp->nrm + pk[6], sp->col, v0->dp);
                p = sp->prim;
                sp->prim = p + sizeof(PFT3);
                ((PFT3 *)p)->xy0 = v0->sxy;
                ((PFT3 *)p)->xy1 = v1->sxy;
                ((PFT3 *)p)->xy2 = v2->sxy;
                ((PFT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PFT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PFT3 *)p)->uv2 = pk[4];
                setlen(p, 7);
                oz = ((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                    gte_strgb(&((PFT3 *)p)->rgbc);
                    ((P_TAG *)p)->code = sp->hdr.b.code;
                }
                break;
            case 0x2C:
                v0 = VTX(pk[9]);
                v1 = VTX(pk[10]);
                v2 = VTX(pk[11]);
                func_800590C4(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                func_8005911C(sp->nrm + pk[8], sp->col, v0->dp);
                p = sp->prim;
                v3 = VTX(pk[12]);
                sp->prim = p + sizeof(PFT4);
                ((PFT4 *)p)->xy0 = v0->sxy;
                ((PFT4 *)p)->xy1 = v1->sxy;
                ((PFT4 *)p)->xy2 = v2->sxy;
                ((PFT4 *)p)->xy3 = v3->sxy;
                ((PFT4 *)p)->uv0 = ((u32 *)pk)[0];
                ((PFT4 *)p)->uv1 = ((u32 *)pk)[1];
                ((PFT4 *)p)->uv2 = pk[4];
                ((PFT4 *)p)->uv3 = pk[6];
                setlen(p, 9);
                oz = (v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                    gte_strgb(&((PFT4 *)p)->rgbc);
                    ((P_TAG *)p)->code = sp->hdr.b.code;
                }
                break;
            case 0x34:
                v0 = VTX(pk[7]);
                v1 = VTX(pk[9]);
                v2 = VTX(pk[11]);
                func_800590C4(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                func_80059140(sp->nrm + pk[6], sp->nrm + pk[8], sp->nrm + pk[10], sp->col, v0->dp);
                p = sp->prim;
                sp->prim = p + sizeof(PGT3);
                ((PGT3 *)p)->xy0 = v0->sxy;
                ((PGT3 *)p)->xy1 = v1->sxy;
                ((PGT3 *)p)->xy2 = v2->sxy;
                ((PGT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PGT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PGT3 *)p)->uv2 = pk[4];
                setlen(p, 9);
                oz = ((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                    gte_strgb3(&((PGT3 *)p)->rgbc0, &((PGT3 *)p)->rgb1, &((PGT3 *)p)->rgb2);
                    ((P_TAG *)p)->code = sp->hdr.b.code;
                }
                break;
            case 0x3C:
                v0 = VTX(pk[9]);
                v1 = VTX(pk[11]);
                v2 = VTX(pk[13]);
                func_800590C4(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                func_80059140(sp->nrm + pk[8], sp->nrm + pk[10], sp->nrm + pk[12], sp->col, v0->dp);
                p = sp->prim;
                v3 = VTX(pk[15]);
                sp->prim = p + sizeof(PGT4);
                ((PGT4 *)p)->xy0 = v0->sxy;
                ((PGT4 *)p)->xy1 = v1->sxy;
                ((PGT4 *)p)->xy2 = v2->sxy;
                ((PGT4 *)p)->xy3 = v3->sxy;
                gte_strgb3(&((PGT4 *)p)->rgbc0, &((PGT4 *)p)->rgb1, &((PGT4 *)p)->rgb2);
                func_8005911C(sp->nrm + pk[14], sp->col, v0->dp);
                ((PGT4 *)p)->uv0 = ((u32 *)pk)[0];
                ((PGT4 *)p)->uv1 = ((u32 *)pk)[1];
                ((PGT4 *)p)->uv2 = pk[4];
                ((PGT4 *)p)->uv3 = pk[6];
                setlen(p, 12);
                ((P_TAG *)p)->code = sp->hdr.b.code;
                oz = (v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                    gte_strgb(&((PGT4 *)p)->rgb3);
                }
                break;
            case 0xA4:
                v0 = VTX(pk[8]);
                p = sp->prim;
                v1 = VTX(pk[9]);
                v2 = VTX(pk[10]);
                sp->prim = p + sizeof(PFT3);
                ((PFT3 *)p)->xy0 = v0->sxy;
                ((PFT3 *)p)->xy1 = v1->sxy;
                ((PFT3 *)p)->xy2 = v2->sxy;
                ((PFT3 *)p)->rgbc = ((u32 *)pk)[3];
                ((PFT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PFT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PFT3 *)p)->uv2 = pk[4];
                setlen(p, 7);
                ((P_TAG *)p)->code = sp->hdr.b.code & 0x7F;
                oz = ((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                }
                break;
            case 0xAC:
                v0 = VTX(pk[10]);
                p = sp->prim;
                v1 = VTX(pk[11]);
                v2 = VTX(pk[12]);
                v3 = VTX(pk[13]);
                sp->prim = p + sizeof(PFT4);
                ((PFT4 *)p)->xy0 = v0->sxy;
                ((PFT4 *)p)->xy1 = v1->sxy;
                ((PFT4 *)p)->xy2 = v2->sxy;
                ((PFT4 *)p)->xy3 = v3->sxy;
                ((PFT4 *)p)->rgbc = ((u32 *)pk)[4];
                ((PFT4 *)p)->uv0 = ((u32 *)pk)[0];
                ((PFT4 *)p)->uv1 = ((u32 *)pk)[1];
                ((PFT4 *)p)->uv2 = pk[4];
                ((PFT4 *)p)->uv3 = pk[6];
                setlen(p, 9);
                ((P_TAG *)p)->code = sp->hdr.b.code & 0x7F;
                oz = (v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                }
                break;
            }
            pk += sp->hdr.b.len * 2;
        } while (sp->n-- != 0);
    }
    D_801A5F24.ctx.buf->cur = sp->prim;
}
