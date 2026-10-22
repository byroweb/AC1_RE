#include "ac1/ac1.h"
/* ovl202 0x8005A57C - draw part `part` of the current model with GTE colour lighting
 * (NCCS/NCCT, no depth cue) using the clip-coded vertex list of func_8005A364:
 * polygons with a vertex behind the eye or entirely off one screen side are dropped,
 * 0x24/0x2C flat lit, 0x34/0x3C gouraud lit (culled through func_80058028), 0xA4/0xAC
 * unlit coloured. */

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ( \
    "lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\t" \
    "lwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\t" \
    "lwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_ncct() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B18043F")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")
#define gte_strgb3(r0, r1, r2) __asm__ volatile ( \
    "swc2 $20, 0(%0)\n\tswc2 $21, 0(%1)\n\tswc2 $22, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")

typedef struct {                /* 0x10 transformed vertex (0x801A6A48, func_8005A364) */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    u8 clip;                    /* +0x06 */
    u8 pad07;
    s32 unk8;
    s32 dp;                     /* +0x0C */
} CVtx;

typedef struct {                /* 0x1C model record */
    SVECTOR *verts;             /* +0x00 */
    u16 nv0;                    /* +0x04 */
    u16 nv1;                    /* +0x06 */
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
    u8 pad0c[0xC];
    union {
        u32 w;
        struct { u8 b0; u8 len; u8 b2; u8 code; } b;
    } hdr;                      /* +0x18 */
    PartRec *rec;               /* +0x1C */
    u8 *nrm;                    /* +0x20 */
    s32 n;                      /* +0x24 */
    u32 *ot;                    /* +0x28 */
    s32 shift4;                 /* +0x2C */
    s32 shift;                  /* +0x30 */
    u8 col[4];                  /* +0x34 */
} CSpad;

typedef struct {                /* 0x801A5F24 */
    DrawCtx ctx;
    u8 pad08[4];
    u8 *model;                  /* +0x0C (0x801A5F30) */
    u8 pad10[0xB24 - 0x10];
    CVtx vtx[1];                /* +0xB24 = 0x801A6A48 */
} DrawWork;

extern DrawWork D_801A5F24;
extern void func_8005A364(s32 n);
extern void func_80058028(s32 *opz, s32 a, s32 b, s32 c);

void func_8005A57C(u16 part, s32 z)
{
    CSpad *sp;
    PartRec *rec;
    CVtx *vtx;
    u16 *pk;
    CVtx *v0;
    CVtx *v1;
    CVtx *v2;
    CVtx *v3;
    u8 *p;
    s32 oz;
    s32 n;

    sp = (CSpad *)0x1F800000;
    sp->col[0] = sp->col[1] = sp->col[2] = 64;
    gte_ldrgb(&((CSpad *)0x1F800000)->col);
    sp->shift4 = 23;
    sp->shift = 11;
    sp->ot = D_801A5F24.ctx.ot + z;
    rec = (PartRec *)(D_801A5F24.model + (part * 32 + 12) - part * 4);
    sp->rec = rec;
    vtx = D_801A5F24.vtx;
    func_8005A364(rec->nv0);
    sp->nrm = sp->rec->nrm;
    n = sp->rec->npkt;
    pk = (u16 *)sp->rec->pkt;
    sp->n = n;
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
            case 0x2C:
                v0 = (CVtx *)((u8 *)vtx + pk[9]);
                v1 = (CVtx *)((u8 *)vtx + pk[10]);
                v2 = (CVtx *)((u8 *)vtx + pk[11]);
                v3 = (CVtx *)((u8 *)vtx + pk[12]);
                if (v0->otz == 0 || v1->otz == 0 || v2->otz == 0 || v3->otz == 0) {
                    break;
                }
                if (v3->clip & (v2->clip & (v0->clip & v1->clip))) {
                    break;
                }
                func_80058028(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                gte_ldv0(sp->nrm + pk[8]);
                gte_nccs();
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
                gte_strgb(&((PFT4 *)p)->rgbc);
                ((P_TAG *)p)->code = sp->hdr.b.code;
                goto quadz;
            case 0x24:
                v0 = (CVtx *)((u8 *)vtx + pk[7]);
                v1 = (CVtx *)((u8 *)vtx + pk[8]);
                v2 = (CVtx *)((u8 *)vtx + pk[9]);
                if (v0->otz == 0 || v1->otz == 0 || v2->otz == 0) {
                    break;
                }
                if (v2->clip & (v0->clip & v1->clip)) {
                    break;
                }
                func_80058028(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                gte_ldv0(sp->nrm + pk[6]);
                gte_nccs();
                p = sp->prim;
                sp->prim = p + sizeof(PFT3);
                ((PFT3 *)p)->xy0 = v0->sxy;
                ((PFT3 *)p)->xy1 = v1->sxy;
                ((PFT3 *)p)->xy2 = v2->sxy;
                ((PFT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PFT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PFT3 *)p)->uv2 = pk[4];
                setlen(p, 7);
                gte_strgb(&((PFT3 *)p)->rgbc);
            tricode:
                ((P_TAG *)p)->code = sp->hdr.b.code;
            triz:
                oz = ((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4;
                addPrim(sp->ot + oz, p);
                break;

            case 0x34:
                v0 = (CVtx *)((u8 *)vtx + pk[7]);
                v1 = (CVtx *)((u8 *)vtx + pk[9]);
                v2 = (CVtx *)((u8 *)vtx + pk[11]);
                if (v0->otz == 0 || v1->otz == 0 || v2->otz == 0) {
                    break;
                }
                if (v2->clip & (v0->clip & v1->clip)) {
                    break;
                }
                func_80058028(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                gte_ldv3(sp->nrm + pk[6], sp->nrm + pk[8], sp->nrm + pk[10]);
                gte_ncct();
                p = sp->prim;
                sp->prim = p + sizeof(PGT3);
                ((PGT3 *)p)->xy0 = v0->sxy;
                ((PGT3 *)p)->xy1 = v1->sxy;
                ((PGT3 *)p)->xy2 = v2->sxy;
                ((PGT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PGT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PGT3 *)p)->uv2 = pk[4];
                setlen(p, 9);
                gte_strgb3(&((PGT3 *)p)->rgbc0, &((PGT3 *)p)->rgb1, &((PGT3 *)p)->rgb2);
                goto tricode;
            case 0x3C:
                v0 = (CVtx *)((u8 *)vtx + pk[9]);
                v1 = (CVtx *)((u8 *)vtx + pk[11]);
                v2 = (CVtx *)((u8 *)vtx + pk[13]);
                v3 = (CVtx *)((u8 *)vtx + pk[15]);
                if (v0->otz == 0 || v1->otz == 0 || v2->otz == 0 || v3->otz == 0) {
                    break;
                }
                if (v3->clip & (v2->clip & (v0->clip & v1->clip))) {
                    break;
                }
                func_80058028(&sp->opz, v0->sxy, v1->sxy, v2->sxy);
                if (sp->opz <= 0) {
                    break;
                }
                gte_ldv3(sp->nrm + pk[8], sp->nrm + pk[10], sp->nrm + pk[12]);
                gte_ncct();
                p = sp->prim;
                sp->prim = p + sizeof(PGT4);
                ((PGT4 *)p)->xy0 = v0->sxy;
                ((PGT4 *)p)->xy1 = v1->sxy;
                ((PGT4 *)p)->xy2 = v2->sxy;
                ((PGT4 *)p)->xy3 = v3->sxy;
                gte_strgb3(&((PGT4 *)p)->rgbc0, &((PGT4 *)p)->rgb1, &((PGT4 *)p)->rgb2);
                gte_ldv0(sp->nrm + pk[14]);
                gte_nccs();
                ((PGT4 *)p)->uv0 = ((u32 *)pk)[0];
                ((PGT4 *)p)->uv1 = ((u32 *)pk)[1];
                ((PGT4 *)p)->uv2 = pk[4];
                ((PGT4 *)p)->uv3 = pk[6];
                setlen(p, 12);
                ((P_TAG *)p)->code = sp->hdr.b.code;
                gte_strgb(&((PGT4 *)p)->rgb3);
            quadz:
                oz = (v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift;
                addPrim(sp->ot + oz, p);
                break;
            case 0xA4:
                v0 = (CVtx *)((u8 *)vtx + pk[8]);
                v1 = (CVtx *)((u8 *)vtx + pk[9]);
                v2 = (CVtx *)((u8 *)vtx + pk[10]);
                if (v0->otz == 0 || v1->otz == 0 || v2->otz == 0) {
                    break;
                }
                if (v2->clip & (v0->clip & v1->clip)) {
                    break;
                }
                p = sp->prim;
                sp->prim = p + sizeof(PFT3);
                ((PFT3 *)p)->xy0 = v0->sxy;
                ((PFT3 *)p)->xy1 = v1->sxy;
                ((PFT3 *)p)->xy2 = v2->sxy;
                ((PFT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PFT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PFT3 *)p)->uv2 = pk[4];
                ((PFT3 *)p)->rgbc = ((u32 *)pk)[3];
                setlen(p, 7);
                ((P_TAG *)p)->code = sp->hdr.b.code & 0x7F;
                goto triz;
            case 0xAC:
                v0 = (CVtx *)((u8 *)vtx + pk[10]);
                v1 = (CVtx *)((u8 *)vtx + pk[11]);
                v2 = (CVtx *)((u8 *)vtx + pk[12]);
                v3 = (CVtx *)((u8 *)vtx + pk[13]);
                if (v0->otz == 0 || v1->otz == 0 || v2->otz == 0 || v3->otz == 0) {
                    break;
                }
                if (v3->clip & (v2->clip & (v0->clip & v1->clip))) {
                    break;
                }
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
                ((PFT4 *)p)->rgbc = ((u32 *)pk)[4];
                setlen(p, 9);
                ((P_TAG *)p)->code = sp->hdr.b.code & 0x7F;
                goto quadz;
            }
            pk += sp->hdr.b.len * 2;
        } while (sp->n-- != 0);
    }
    D_801A5F24.ctx.buf->cur = sp->prim;
}
