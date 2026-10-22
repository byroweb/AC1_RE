#include "ac1/ac1.h"
#include "ac1/prim2d.h"

typedef struct {                /* 0x10 */
    s32 sxy;
    s16 sz;
    u8 pad06[0x0A];
} PVert;

typedef struct {
    u32 *vert;                  /* +0x00 */
    u16 n_vert;                 /* +0x04 */
    u8 pad06[0x0A];
    u32 *prim;                  /* +0x10 */
    u16 n_prim;                 /* +0x14 */
} MdlObj;

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
        u8 c[4];
    } rgb;                      /* +0x2C */
} DrawScr;

typedef struct {
    u32 tag;
    u32 rgbc;                   /* +0x04 */
    s32 xy[4];                  /* +0x08 */
} PolyF4w;

extern s32 D_80194734;
typedef struct {
    PrimBuf *buf;               /* 0x8019D95C */
    u32 *ot;                    /* 0x8019D960 */
    u8 pad08[0x66C];
    u8 verts[1];                /* 0x8019DFD0 PVert, indexed by byte offset */
} RenderCtx;
extern RenderCtx D_8019D95C;
extern void func_80057964(s32 n);

void func_800573FC(MdlObj *obj, s32 ot)
{
    DrawScr *s;
    u32 *p;
    PolyF4w *pr;
    PVert *v0;
    PVert *v1;
    PVert *v2;
    PVert *v3;
    s32 z;
    u8 *vt;

    s = (DrawScr *)0x1F800000;
    s->rgb.c[0] = s->rgb.c[1] = s->rgb.c[2] = 10;
    s->unk28 = 0;
    s->obj = obj;
    s->far = D_80194734 + 15;
    s->shift = D_80194734 + 3;
    s->ot = D_8019D95C.ot + ot;
    vt = D_8019D95C.verts;
    func_80057964(obj->n_vert);
    p = s->obj->prim;
    s->count = s->obj->n_prim;
    s->cur = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    if (s->end < s->cur + (s->count << 2) + (s->count << 4) + (s->count << 5)) {
        return;
    }
    while (s->count-- != 0) {
        s->tag.w = *p++;
        if ((s->tag.b.code & 0xFC) == 0x28) {
            pr = (PolyF4w *)s->cur;
            s->cur += 0x18;
            v0 = (PVert *)(vt + ((u16 *)p)[3]);
            v1 = (PVert *)(vt + ((u16 *)p)[4]);
            v2 = (PVert *)(vt + ((u16 *)p)[5]);
            v3 = (PVert *)(vt + ((u16 *)p)[6]);
            pr->xy[0] = v0->sxy;
            pr->xy[1] = v1->sxy;
            pr->xy[2] = v2->sxy;
            pr->xy[3] = v3->sxy;
            pr->rgbc = s->rgb.w;
            ((u8 *)pr)[7] = s->tag.b.code;
            ((u8 *)pr)[3] = 5;
            z = (v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift;
            if (z >= 0) {
                addPrim(s->ot + z, pr);
            }
        }
        p += s->tag.b.len;
    }
    D_8019D95C.buf->cur = s->cur;
}
