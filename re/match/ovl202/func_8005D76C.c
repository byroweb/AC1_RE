#include "ac1/ac1.h"
/* ovl202 0x8005D76C - draw model part `part` of the current model (D_801A5F30.model):
 * textured triangles (packet code 0xA4) as POLY_FT3 and quads (0xAC) as POLY_FT4 with
 * colour *col, from the vertex list transformed by func_80057F74 (0x801A6A48). */

typedef struct {
    s32 sxy;
    s16 otz;
    s16 pad06;
    s32 unk8;
    s32 dp;
} ScrVtx2;

typedef struct {                /* 0x1C */
    u8 pad00[4];
    u16 nv0;                    /* +0x04 */
    u16 nv1;                    /* +0x06 */
    u8 pad08[6];
    u16 flags;                  /* +0x0E low 9 bits: extra packet count */
    u32 *pkt;                   /* +0x10 */
    u16 npkt;                   /* +0x14 */
    u8 pad16[6];
} PartRec;

typedef struct {
    u8 r, g, b, code;
} Col4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;
    u32 uv0;
    s32 xy1;
    u32 uv1;
    s32 xy2;
    u16 uv2;
    u16 pad;
} PolyFT3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;
    u32 uv0;
    s32 xy1;
    u32 uv1;
    s32 xy2;
    u16 uv2;
    u16 pad1;
    s32 xy3;
    u16 uv3;
    u16 pad2;
} PolyFT4;

typedef struct {
    u8 pad00[4];
    u8 *prim;                   /* +0x04 */
    u8 *end;                    /* +0x08 */
    union {
        u32 w;
        struct { u8 b0, len, b2, code; } b;
    } hdr;                      /* +0x0C */
    PartRec *rec;               /* +0x10 */
    u8 pad14[4];
    s32 n;                      /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 shift4;                 /* +0x20 */
    s32 shift;                  /* +0x24 */
    s32 zmin;                   /* +0x28 */
    Col4 col;                   /* +0x2C */
} Spad;

typedef struct {                /* 0x801A5F24 */
    DrawCtx ctx;
    u8 *model;                  /* +0x08 (0x801A5F2C..) */
    u8 pad0c[0xB24 - 0xC];
    ScrVtx2 vtx[1];             /* +0xB24 = 0x801A6A48 */
} DrawWork;

extern DrawWork D_801A5F24;
extern u8 *D_801A5F30;
extern s32 D_801987CC;
void func_80057F74(s32 n);

void func_8005D76C(s32 part, Col4 *col, s32 z)
{
    Spad *sp;
    u8 *p;
    ScrVtx2 *vtx;
    u16 *pk;
    ScrVtx2 *v0;
    ScrVtx2 *v1;
    ScrVtx2 *v2;
    ScrVtx2 *v3;
    s32 n;
    s32 oz;

    sp = (Spad *)0x1F800000;
    sp->col = *col;
    sp->zmin = 0;
    sp->shift4 = D_801987CC + 15;
    sp->shift = D_801987CC + 3;
    sp->ot = D_801A5F24.ctx.ot + z;
    sp->rec = (PartRec *)(D_801A5F30 + (part * 32 + 12) - part * 4);
    vtx = D_801A5F24.vtx;
    func_80057F74(sp->rec->nv0 + sp->rec->nv1);
    n = sp->rec->npkt + (sp->rec->flags & 0x1FF);
    sp->n = n;
    pk = (u16 *)sp->rec->pkt;
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
            switch (sp->hdr.b.code & 0xFC) {
            case 0xA4:
                v0 = (ScrVtx2 *)((u8 *)vtx + pk[8]);
                v1 = (ScrVtx2 *)((u8 *)vtx + pk[9]);
                v2 = (ScrVtx2 *)((u8 *)vtx + pk[10]);
                p = sp->prim;
                sp->prim = p + sizeof(PolyFT3);
                ((PolyFT3 *)p)->xy0 = v0->sxy;
                ((PolyFT3 *)p)->xy1 = v1->sxy;
                ((PolyFT3 *)p)->xy2 = v2->sxy;
                *(u32 *)&((PolyFT3 *)p)->r0 = *(u32 *)&sp->col;
                ((PolyFT3 *)p)->uv0 = ((u32 *)pk)[0];
                ((PolyFT3 *)p)->uv1 = ((u32 *)pk)[1];
                ((PolyFT3 *)p)->uv2 = pk[4];
                setlen(p, 7);
                ((PolyFT3 *)p)->code = sp->hdr.b.code & 0x7F;
                oz = ((v0->otz + v1->otz + v2->otz) * 0x1555) >> sp->shift4;
                if (sp->zmin < oz) {
                    addPrim(sp->ot + oz, p);
                }
                break;
            case 0xAC:
                v0 = (ScrVtx2 *)((u8 *)vtx + pk[10]);
                v1 = (ScrVtx2 *)((u8 *)vtx + pk[11]);
                v2 = (ScrVtx2 *)((u8 *)vtx + pk[12]);
                v3 = (ScrVtx2 *)((u8 *)vtx + pk[13]);
                p = sp->prim;
                sp->prim = p + sizeof(PolyFT4);
                ((PolyFT4 *)p)->xy0 = v0->sxy;
                ((PolyFT4 *)p)->xy1 = v1->sxy;
                ((PolyFT4 *)p)->xy2 = v2->sxy;
                ((PolyFT4 *)p)->xy3 = v3->sxy;
                *(u32 *)&((PolyFT4 *)p)->r0 = *(u32 *)&sp->col;
                ((PolyFT4 *)p)->uv0 = ((u32 *)pk)[0];
                ((PolyFT4 *)p)->uv1 = ((u32 *)pk)[1];
                ((PolyFT4 *)p)->uv2 = pk[4];
                ((PolyFT4 *)p)->uv3 = pk[6];
                setlen(p, 9);
                ((PolyFT4 *)p)->code = sp->hdr.b.code & 0x7F;
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
