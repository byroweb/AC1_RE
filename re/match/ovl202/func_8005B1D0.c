#include "ac1/ac1.h"
/* ovl202 0x8005B1D0 - draw a model record lit by the GTE: flat-lit textured quads /
 * triangles (0x2C/0x24, culled), two-sided quads (0x6C), and per-vertex palette
 * coloured gouraud quads / triangles (0x2D/0x25, colour index in the source vertex
 * pad, palette `pal` of D_80090A28); OT slot from the average z plus the packet bias. */

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x10 transformed vertex (0x801A6A48) */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    u8 pad06[2];
    u8 col[3];                  /* +0x08 */
    u8 clip;                    /* +0x0B */
    s32 dp;                     /* +0x0C */
} LVtx;

typedef struct {                /* 0x1C model record */
    SVECTOR *verts;             /* +0x00 */
    u16 nv;                     /* +0x04 */
    u8 pad06[2];
    u8 *nrm;                    /* +0x08 */
    u8 pad0c[4];
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
        struct { s8 bias; u8 len; u8 b2; u8 code; } b;
    } hdr;                      /* +0x0C */
    u8 *nrm;                    /* +0x10 */
    SVECTOR *verts;             /* +0x14 */
    s32 n;                      /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 shift4;                 /* +0x20 */
    s32 shift;                  /* +0x24 */
    u8 *pal;                    /* +0x28 */
    u8 col[4];                  /* +0x2C */
} PSpad;

typedef struct {                /* 0x801A5F24 */
    DrawCtx ctx;
    u8 pad08[0xB24 - 0x8];
    LVtx vtx[1];                /* +0xB24 = 0x801A6A48 */
} DrawWork;

extern DrawWork D_801A5F24;
extern s32 D_801987CC;
extern u8 D_80090A28[];
extern void func_8005B124(SVECTOR *v, s32 n);
extern void func_8005BDBC(PSpad *sp, s32 a, s32 b, s32 c);
extern void func_8005BDE0(u8 *nrm, u8 *col, s32 dp);

#define VCOL(v, i) gte_ldrgb(sp->pal + ((SVECTOR *)((u8 *)sp->verts + (pk[i] >> 1)))->pad); gte_lddp((v)->dp); gte_ncds()

void func_8005B1D0(PartRec *rec, s32 pal, s32 z)
{
    PSpad *sp;
    LVtx *vtx;
    u16 *pk;
    LVtx *v0;
    LVtx *v1;
    LVtx *v2;
    LVtx *v3;
    u8 *p;
    s32 oz;

    sp = (PSpad *)0x1F800000;
    sp->n = rec->npkt;
    if (rec->npkt == 0) {
        return;
    }
    sp->col[0] = sp->col[1] = sp->col[2] = 64;
    sp->shift4 = D_801987CC + 15;
    sp->shift = D_801987CC + 3;
    sp->ot = D_801A5F24.ctx.ot + z;
    vtx = D_801A5F24.vtx;
    func_8005B124(rec->verts, rec->nv);
    sp->verts = rec->verts;
    sp->nrm = rec->nrm;
    sp->pal = D_80090A28 + (pal << 7);
    pk = (u16 *)rec->pkt;
    sp->prim = D_801A5F24.ctx.buf->cur;
    sp->end = D_801A5F24.ctx.buf->end;
    do {
        sp->hdr.w = *(u32 *)pk;
        pk += 2;
        switch (sp->hdr.b.code & 0xFD) {
        case 0x2C:
            v0 = (LVtx *)((u8 *)vtx + pk[9]);
            if (v0->otz == 0) {
                break;
            }
            v1 = (LVtx *)((u8 *)vtx + pk[10]);
            if (v1->otz == 0) {
                break;
            }
            v2 = (LVtx *)((u8 *)vtx + pk[11]);
            if (v2->otz == 0) {
                break;
            }
            v3 = (LVtx *)((u8 *)vtx + pk[12]);
            if (v3->otz == 0) {
                break;
            }
            func_8005BDBC(sp, v0->sxy, v1->sxy, v2->sxy);
            if (sp->opz < 0) {
                break;
            }
            func_8005BDE0(sp->nrm + pk[8], sp->col, v0->dp);
            p = sp->prim;
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
            oz = ((v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift) + sp->hdr.b.bias * 4;
            if (oz >= 0) {
                addPrim(sp->ot + oz, p);
                gte_strgb(&((PFT4 *)p)->rgbc);
                ((P_TAG *)p)->code = sp->hdr.b.code;
            }
            break;
        case 0x24:
            v0 = (LVtx *)((u8 *)vtx + pk[7]);
            if (v0->otz == 0) {
                break;
            }
            v1 = (LVtx *)((u8 *)vtx + pk[8]);
            if (v1->otz == 0) {
                break;
            }
            v2 = (LVtx *)((u8 *)vtx + pk[9]);
            if (v2->otz == 0) {
                break;
            }
            func_8005BDBC(sp, v0->sxy, v1->sxy, v2->sxy);
            if (sp->opz < 0) {
                break;
            }
            func_8005BDE0(sp->nrm + pk[6], sp->col, v0->dp);
            p = sp->prim;
            sp->prim = p + sizeof(PFT3);
            ((PFT3 *)p)->xy0 = v0->sxy;
            ((PFT3 *)p)->xy1 = v1->sxy;
            ((PFT3 *)p)->xy2 = v2->sxy;
            ((PFT3 *)p)->uv0 = ((u32 *)pk)[0];
            ((PFT3 *)p)->uv1 = ((u32 *)pk)[1];
            ((PFT3 *)p)->uv2 = pk[4];
            setlen(p, 7);
            oz = (((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4) + sp->hdr.b.bias * 4;
            if (oz >= 0) {
                addPrim(sp->ot + oz, p);
                gte_strgb(&((PFT3 *)p)->rgbc);
                ((P_TAG *)p)->code = sp->hdr.b.code;
            }
            break;
        case 0x2D:
            v0 = (LVtx *)((u8 *)vtx + pk[9]);
            if (v0->otz == 0) {
                break;
            }
            v1 = (LVtx *)((u8 *)vtx + pk[10]);
            if (v1->otz == 0) {
                break;
            }
            v2 = (LVtx *)((u8 *)vtx + pk[11]);
            if (v2->otz == 0) {
                break;
            }
            v3 = (LVtx *)((u8 *)vtx + pk[12]);
            if (v3->otz == 0) {
                break;
            }
            func_8005BDBC(sp, v0->sxy, v1->sxy, v2->sxy);
            if (sp->opz < 0) {
                break;
            }
            gte_ldv0(sp->nrm + pk[8]);
            VCOL(v0, 9);
            p = sp->prim;
            sp->prim = p + sizeof(PGT4);
            gte_strgb(&((PGT4 *)p)->rgbc0);
            VCOL(v1, 10);
            ((PGT4 *)p)->xy0 = v0->sxy;
            ((PGT4 *)p)->xy1 = v1->sxy;
            ((PGT4 *)p)->xy2 = v2->sxy;
            ((PGT4 *)p)->xy3 = v3->sxy;
            gte_strgb(&((PGT4 *)p)->rgb1);
            VCOL(v2, 11);
            ((PGT4 *)p)->uv0 = ((u32 *)pk)[0];
            ((PGT4 *)p)->uv1 = ((u32 *)pk)[1];
            ((PGT4 *)p)->uv2 = pk[4];
            ((PGT4 *)p)->uv3 = pk[6];
            gte_strgb(&((PGT4 *)p)->rgb2);
            VCOL(v3, 12);
            setlen(p, 12);
            ((P_TAG *)p)->code = (sp->hdr.b.code & 2) | 0x3C;
            oz = ((v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift) + sp->hdr.b.bias * 4;
            if (oz >= 0) {
                addPrim(sp->ot + oz, p);
                gte_strgb(&((PGT4 *)p)->rgb3);
            }
            break;
        case 0x25:
            v0 = (LVtx *)((u8 *)vtx + pk[7]);
            if (v0->otz == 0) {
                break;
            }
            v1 = (LVtx *)((u8 *)vtx + pk[8]);
            if (v1->otz == 0) {
                break;
            }
            v2 = (LVtx *)((u8 *)vtx + pk[9]);
            if (v2->otz == 0) {
                break;
            }
            func_8005BDBC(sp, v0->sxy, v1->sxy, v2->sxy);
            if (sp->opz < 0) {
                break;
            }
            gte_ldv0(sp->nrm + pk[6]);
            VCOL(v0, 7);
            p = sp->prim;
            sp->prim = p + sizeof(PGT3);
            gte_strgb(&((PGT3 *)p)->rgbc0);
            VCOL(v1, 8);
            ((PGT3 *)p)->xy0 = v0->sxy;
            ((PGT3 *)p)->xy1 = v1->sxy;
            ((PGT3 *)p)->xy2 = v2->sxy;
            gte_strgb(&((PGT3 *)p)->rgb1);
            VCOL(v2, 9);
            ((PGT3 *)p)->uv0 = ((u32 *)pk)[0];
            ((PGT3 *)p)->uv1 = ((u32 *)pk)[1];
            ((PGT3 *)p)->uv2 = pk[4];
            setlen(p, 9);
            ((P_TAG *)p)->code = (sp->hdr.b.code & 2) | 0x34;
            oz = (((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4) + sp->hdr.b.bias * 4;
            if (oz >= 0) {
                addPrim(sp->ot + oz, p);
                gte_strgb(&((PGT3 *)p)->rgb2);
            }
            break;
        case 0x6C:
            v0 = (LVtx *)((u8 *)vtx + pk[9]);
            if (v0->otz == 0) {
                break;
            }
            func_8005BDE0(sp->nrm + pk[8], sp->col, v0->dp);
            p = sp->prim;
            sp->prim = p + sizeof(PFT4);
            v1 = (LVtx *)((u8 *)vtx + pk[10]);
            if (v1->otz == 0) {
                break;
            }
            v2 = (LVtx *)((u8 *)vtx + pk[11]);
            if (v2->otz == 0) {
                break;
            }
            v3 = (LVtx *)((u8 *)vtx + pk[12]);
            if (v3->otz == 0) {
                break;
            }
            ((PFT4 *)p)->xy0 = v0->sxy;
            ((PFT4 *)p)->xy1 = v1->sxy;
            ((PFT4 *)p)->xy2 = v2->sxy;
            ((PFT4 *)p)->xy3 = v3->sxy;
            ((PFT4 *)p)->uv0 = ((u32 *)pk)[0];
            ((PFT4 *)p)->uv1 = ((u32 *)pk)[1];
            ((PFT4 *)p)->uv2 = pk[4];
            ((PFT4 *)p)->uv3 = pk[6];
            setlen(p, 9);
            oz = ((v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift) + sp->hdr.b.bias * 4;
            if (oz >= 0) {
                addPrim(sp->ot + oz, p);
                gte_strgb(&((PFT4 *)p)->rgbc);
                ((P_TAG *)p)->code = sp->hdr.b.code & 0xBF;
            }
            break;
        }
        pk += sp->hdr.b.len * 2;
    } while (--sp->n != 0);
    D_801A5F24.ctx.buf->cur = sp->prim;
}
