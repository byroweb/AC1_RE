#include "ac1/ac1.h"
#include "ac1/prim2d.h"

typedef struct {                /* 0x10 */
    s32 sxy;
    s16 sz;
    u8 pad06[0x0A];
} PVert;

typedef struct {                /* 0x1C */
    u32 *vert;                  /* +0x00 */
    u16 n_vert;                 /* +0x04 */
    u16 n_vert2;                /* +0x06 */
    u8 pad08[6];
    u16 n_extra;                /* +0x0E (& 0x1FF) */
    u32 *prim;                  /* +0x10 */
    u16 n_prim;                 /* +0x14 */
    u8 pad16[6];
} MdlObj;

typedef struct {
    u8 hdr[0x0C];
    MdlObj obj[1];              /* +0x0C */
} Model;

typedef struct {
    u8 r, g, b, cd;
} Col4;

typedef struct {                /* scratchpad 0x1F800000 */
    u8 pad00[4];
    u8 *cur;                    /* +0x04 */
    u8 *end;                    /* +0x08 */
    union {
        u32 w;
        struct {
            u8 b0;
            u8 len;             /* +0x0D */
            u8 b2;
            u8 code;            /* +0x0F */
        } b;
    } tag;                      /* +0x0C */
    MdlObj *obj;                /* +0x10 */
    u8 pad14[4];
    s32 count;                  /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 far;                    /* +0x20 */
    s32 shift;                  /* +0x24 */
    s32 unk28;                  /* +0x28 */
    union {
        u32 w;
        Col4 c;
    } rgb;                      /* +0x2C */
} DrawScr;

typedef struct {                /* POLY_FT3 */
    u32 tag;
    u32 rgbc;                   /* +0x04 */
    s32 xy0;                    /* +0x08 */
    u32 uv0;                    /* +0x0C uv0 + clut */
    s32 xy1;                    /* +0x10 */
    u32 uv1;                    /* +0x14 uv1 + tpage */
    s32 xy2;                    /* +0x18 */
    u16 uv2;                    /* +0x1C */
    u16 pad1e;
} PolyFT3w;

typedef struct {                /* POLY_FT4 */
    u32 tag;
    u32 rgbc;                   /* +0x04 */
    s32 xy0;                    /* +0x08 */
    u32 uv0;                    /* +0x0C */
    s32 xy1;                    /* +0x10 */
    u32 uv1;                    /* +0x14 */
    s32 xy2;                    /* +0x18 */
    u16 uv2;                    /* +0x1C */
    u16 pad1e;
    s32 xy3;                    /* +0x20 */
    u16 uv3;                    /* +0x24 */
    u16 pad26;
} PolyFT4w;

extern s32 D_80194734;
typedef struct {
    PrimBuf *buf;               /* 0x8019D95C */
    u32 *ot;                    /* 0x8019D960 */
    u8 pad08[4];
    Model *model;               /* 0x8019D968 */
    u8 pad10[0x664];
    u8 verts[1];                /* 0x8019DFD0 PVert, indexed by byte offset */
} RenderCtx;
extern RenderCtx D_8019D95C;
extern void func_80057964(s32 n);
extern u8 D_TODO[];

void func_8005D15C(s32 idx, Col4 *col, s32 ot)
{
    DrawScr *s;
    u32 *p;
    PolyFT4w *q;
    PVert *v0;
    PVert *v1;
    PVert *v2;
    PVert *v3;
    s32 z;
    u8 *vt;
    MdlObj *obj;

    s = (DrawScr *)0x1F800000;
    s->rgb.c = *col;
    s->unk28 = 0;
    s->far = D_80194734 + 15;
    s->shift = D_80194734 + 3;
    s->ot = D_8019D95C.ot + ot;
    vt = D_8019D95C.verts;
    obj = (MdlObj *)((u8 *)D_8019D95C.model + (idx * 32 + 12) - idx * 4);
    s->obj = obj;
    func_80057964(obj->n_vert + obj->n_vert2);
    s->count = s->obj->n_prim + (s->obj->n_extra & 0x1FF);
    p = s->obj->prim;
    s->cur = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    if (s->end < s->cur + (s->count << 2) + (s->count << 4) + (s->count << 5)) {
        return;
    }
    while (s->count-- != 0) {
        s->tag.w = *p++;
        switch (s->tag.b.code & 0xFC) {
        case 0xA4:
            v0 = (PVert *)(vt + ((u16 *)p)[8]);
            q = (PolyFT4w *)s->cur;
            v1 = (PVert *)(vt + ((u16 *)p)[9]);
            v2 = (PVert *)(vt + ((u16 *)p)[10]);
            s->cur += 0x20;
            q->xy0 = v0->sxy;
            q->xy1 = v1->sxy;
            q->xy2 = v2->sxy;
            q->rgbc = s->rgb.w;
            q->uv0 = p[0];
            q->uv1 = p[1];
            q->uv2 = ((u16 *)p)[4];
            ((u8 *)q)[3] = 7;
            ((u8 *)q)[7] = s->tag.b.code & 0x7F;
            z = ((v0->sz + v1->sz + v2->sz) * 0x1555) >> s->far;
            if (s->unk28 < z) {
                addPrim(s->ot + z, q);
            }
            break;
        case 0xAC:
            v0 = (PVert *)(vt + ((u16 *)p)[10]);
            q = (PolyFT4w *)s->cur;
            v1 = (PVert *)(vt + ((u16 *)p)[11]);
            v2 = (PVert *)(vt + ((u16 *)p)[12]);
            v3 = (PVert *)(vt + ((u16 *)p)[13]);
            s->cur += 0x28;
            q->xy0 = v0->sxy;
            q->xy1 = v1->sxy;
            q->xy2 = v2->sxy;
            q->xy3 = v3->sxy;
            q->rgbc = s->rgb.w;
            q->uv0 = p[0];
            q->uv1 = p[1];
            q->uv2 = ((u16 *)p)[4];
            q->uv3 = ((u16 *)p)[6];
            ((u8 *)q)[3] = 9;
            ((u8 *)q)[7] = s->tag.b.code & 0x7F;
            z = (v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift;
            if (s->unk28 < z) {
                addPrim(s->ot + z, q);
            }
            break;
        }
        p += s->tag.b.len;
    }
    D_8019D95C.buf->cur = s->cur;
}
