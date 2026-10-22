#include "ac1/ac1.h"
/* ovl202 0x80057A00 - draw the flat quads (code 0x28..0x2B) of a model's packet list as
 * POLY_F4, colour 10,10,10, from the transformed vertex list D_801A6A48 (func_80057F74). */

typedef struct {
    s32 sxy;
    s32 otz;
    s32 unk8;
    s32 dp;
} ScrVtx;

typedef struct {
    u8 pad00[4];
    u16 nvert;                  /* +0x04 */
    u8 pad06[0x0A];
    u32 *pkt;                   /* +0x10 */
    u16 npkt;                   /* +0x14 */
} PktModel;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;
    s32 xy1;
    s32 xy2;
    s32 xy3;
} PolyF4;

typedef struct {
    u8 pad00[4];
    u8 *prim;                   /* +0x04 */
    u8 *end;                    /* +0x08 */
    union {
        u32 w;
        struct { u8 b0, len, b2, code; } b;
    } hdr;                      /* +0x0C */
    PktModel *model;            /* +0x10 */
    u8 pad14[4];
    s32 n;                      /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 shift4;                 /* +0x20 */
    s32 shift;                  /* +0x24 */
    s32 unk28;                  /* +0x28 */
    u8 r, g, b, code;           /* +0x2C */
} Spad;

typedef struct {
    s32 sxy;
    s16 otz;
    s16 pad06;
    s32 unk8;
    s32 dp;
} ScrVtx2;

typedef struct {                /* 0x801A5F24 */
    DrawCtx ctx;
    u8 pad08[0xB24 - 8];
    ScrVtx2 vtx[1];             /* +0xB24 = 0x801A6A48 */
} DrawWork;

extern DrawWork D_801A5F24;
extern s32 D_801987CC;
void func_80057F74(s32 n);

void func_80057A00(PktModel *m, s32 z)
{
    Spad *sp;
    PolyF4 *p;
    ScrVtx2 *vtx;
    u16 *pk;
    ScrVtx2 *v0;
    ScrVtx2 *v1;
    ScrVtx2 *v2;
    ScrVtx2 *v3;
    s32 n;
    s32 oz;

    sp = (Spad *)0x1F800000;
    sp->b = 10;
    sp->g = 10;
    sp->r = 10;
    sp->unk28 = 0;
    sp->model = m;
    sp->shift4 = D_801987CC + 15;
    sp->shift = D_801987CC + 3;
    sp->ot = D_801A5F24.ctx.ot + z;
    vtx = D_801A5F24.vtx;
    func_80057F74(m->nvert);
    n = sp->model->npkt;
    pk = (u16 *)sp->model->pkt;
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
            if ((sp->hdr.b.code & 0xFC) == 0x28) {
                p = (PolyF4 *)sp->prim;
                sp->prim = (u8 *)(p + 1);
                v0 = (ScrVtx2 *)((u8 *)vtx + pk[3]);
                v1 = (ScrVtx2 *)((u8 *)vtx + pk[4]);
                v2 = (ScrVtx2 *)((u8 *)vtx + pk[5]);
                v3 = (ScrVtx2 *)((u8 *)vtx + pk[6]);
                p->xy0 = v0->sxy;
                p->xy1 = v1->sxy;
                p->xy2 = v2->sxy;
                p->xy3 = v3->sxy;
                *(u32 *)&p->r0 = *(u32 *)&sp->r;
                p->code = sp->hdr.b.code;
                setlen(p, 5);
                oz = (v0->otz + v1->otz + v2->otz + v3->otz) >> sp->shift;
                if (oz >= 0) {
                    addPrim(sp->ot + oz, p);
                }
            }
            pk += sp->hdr.b.len * 2;
        } while (sp->n-- != 0);
    }
    D_801A5F24.ctx.buf->cur = sp->prim;
}
