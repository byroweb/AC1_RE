#include "ac1/ac1.h"
#include "ac1/gte.h"
#include "ac1/prim2d.h"

#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_ncs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_nct() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B18043F")
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")
#define gte_strgb3(r0, r1, r2) __asm__ volatile ("swc2 $20, 0(%0)\n\tswc2 $21, 0(%1)\n\tswc2 $22, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")

typedef struct {                /* 0x08 transformed vertex */
    s32 sxy;                    /* +0x00 */
    s16 sz;                     /* +0x04 */
    u8 clip;                    /* +0x06 */
    u8 pad07;
} PVert;

typedef struct {                /* 0x1C */
    u32 *vert;                  /* +0x00 */
    u16 n_vert;                 /* +0x04 */
    u8 pad06[2];
    u8 *normals;                /* +0x08 */
    u8 pad0c[4];
    u32 *prim;                  /* +0x10 */
    u16 n_prim;                 /* +0x14 */
    u8 pad16[6];
} MdlObj;

typedef struct {                /* scratchpad 0x1F800000 */
    s32 clip;                   /* +0x00 */
    u8 *cur;                    /* +0x04 */
    u8 *end;                    /* +0x08 */
    u8 pad0c[0x0C];
    union {
        u32 w;
        struct {
            u8 b0;
            u8 len;             /* +0x19 */
            u8 b2;
            u8 code;            /* +0x1B */
        } b;
    } tag;                      /* +0x18 */
    MdlObj *obj;                /* +0x1C */
    u8 *normals;                /* +0x20 */
    s32 count;                  /* +0x24 */
    u32 *ot;                    /* +0x28 */
    s32 far;                    /* +0x2C */
    s32 shift;                  /* +0x30 */
    u8 rgb[4];                  /* +0x34 */
} NcsScr;

typedef struct {
    PrimBuf *buf;               /* 0x8019D95C */
    u32 *ot;                    /* 0x8019D960 */
    u8 pad08[4];
    u8 *model;                  /* 0x8019D968 */
    u8 pad0c[0x664];
    u8 verts[1];                /* 0x8019DFD0 */
} RenderCtx;

extern RenderCtx D_8019D95C;
extern void func_80059D54(s32 n);
extern void func_80057A18(NcsScr *s, s32 a, s32 b, s32 c);

#define D(i) (((u16 *)p)[i])
typedef struct { unsigned addr : 24; unsigned len : 8; unsigned rgb : 24; unsigned code : 8; } PT2;
#define setcode2(p, c) (((PT2 *)(p))->code = (c))
#define ADDPRIM(q, z) \
    *(u32 *)(q) = (*(u32 *)(q) & 0xFF000000) | (s->ot[z] & 0xFFFFFF); \
    s->ot[z] = (s->ot[z] & 0xFF000000) | ((u32)(q) & 0xFFFFFF)

void func_80059F6C(u16 idx, s32 ot)
{
    NcsScr *s;
    MdlObj *obj;
    u8 *vt;
    u32 *p;
    u8 *q;
    PVert *v0;
    PVert *v1;
    PVert *v2;
    PVert *v3;
    s32 z;
    u32 tag;

    s = (NcsScr *)0x1F800000;
    s->rgb[0] = s->rgb[1] = s->rgb[2] = 64;
    gte_ldrgb(s->rgb);
    s->far = 23;
    s->shift = 11;
    s->ot = D_8019D95C.ot + ot;
    obj = (MdlObj *)(D_8019D95C.model + (idx * 32 + 12) - idx * 4);
    s->obj = obj;
    vt = D_8019D95C.verts;
    func_80059D54(obj->n_vert);
    s->normals = s->obj->normals;
    p = s->obj->prim;
    s->count = s->obj->n_prim;
    s->cur = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    if (s->end < s->cur + (s->count << 2) + (s->count << 4) + (s->count << 5)) {
        return;
    }
    while (s->count-- != 0) {
        s->tag.w = *p++;
        switch (s->tag.b.code & 0xFD) {
        case 0x2C:
            v0 = (PVert *)(vt + D(9));
            v1 = (PVert *)(vt + D(10));
            v2 = (PVert *)(vt + D(11));
            v3 = (PVert *)(vt + D(12));
            if (v0->sz == 0 || v1->sz == 0 || v2->sz == 0 || v3->sz == 0) break;
            if (v3->clip & (v2->clip & (v0->clip & v1->clip))) break;
            func_80057A18(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip <= 0) break;
            gte_ldv0(s->normals + D(8));
            gte_ncs();
            q = s->cur;
            s->cur = q + 40;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(s32 *)(q + 32) = v3->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            *(u16 *)(q + 36) = D(6);
            setlen(q, 9);
            gte_strgb(q + 4);
            setcode(q, s->tag.b.code);
            goto quad;
        case 0x24:
            v0 = (PVert *)(vt + D(7));
            v1 = (PVert *)(vt + D(8));
            v2 = (PVert *)(vt + D(9));
            if (v0->sz == 0 || v1->sz == 0 || v2->sz == 0) break;
            if (v2->clip & (v0->clip & v1->clip)) break;
            func_80057A18(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip <= 0) break;
            gte_ldv0(s->normals + D(6));
            gte_ncs();
            q = s->cur;
            s->cur = q + 32;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            setlen(q, 7);
            gte_strgb(q + 4);
            setcode(q, s->tag.b.code);
        tri:
            z = ((v0->sz + v1->sz + v2->sz) * 0x1555) >> s->far;
            addPrim(s->ot + z, q);
            break;
        case 0x34:
            v0 = (PVert *)(vt + D(7));
            v1 = (PVert *)(vt + D(9));
            v2 = (PVert *)(vt + D(11));
            if (v0->sz == 0 || v1->sz == 0 || v2->sz == 0) break;
            if (v2->clip & (v0->clip & v1->clip)) break;
            func_80057A18(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip <= 0) break;
            gte_ldv3(s->normals + D(6), s->normals + D(8), s->normals + D(10));
            gte_nct();
            q = s->cur;
            s->cur = q + 40;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 20) = v1->sxy;
            *(s32 *)(q + 32) = v2->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 24) = p[1];
            *(u16 *)(q + 36) = D(4);
            setlen(q, 9);
            gte_strgb3(q + 4, q + 16, q + 28);
            setcode(q, s->tag.b.code);
            goto tri;
        case 0x3C:
            v0 = (PVert *)(vt + D(9));
            v1 = (PVert *)(vt + D(11));
            v2 = (PVert *)(vt + D(13));
            v3 = (PVert *)(vt + D(15));
            if (v0->sz == 0 || v1->sz == 0 || v2->sz == 0 || v3->sz == 0) break;
            if (v3->clip & (v2->clip & (v0->clip & v1->clip))) break;
            func_80057A18(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip <= 0) break;
            gte_ldv3(s->normals + D(8), s->normals + D(10), s->normals + D(12));
            gte_nct();
            q = s->cur;
            s->cur = q + 52;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 20) = v1->sxy;
            *(s32 *)(q + 32) = v2->sxy;
            *(s32 *)(q + 44) = v3->sxy;
            gte_strgb3(q + 4, q + 16, q + 28);
            gte_ldv0(s->normals + D(14));
            gte_ncs();
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 24) = p[1];
            *(u16 *)(q + 36) = D(4);
            *(u16 *)(q + 48) = D(6);
            setlen(q, 12);
            setcode(q, s->tag.b.code);
            gte_strgb(q + 40);
        quad:
            z = (v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift;
            addPrim(s->ot + z, q);
            break;
        case 0xA4:
            v0 = (PVert *)(vt + D(8));
            v1 = (PVert *)(vt + D(9));
            v2 = (PVert *)(vt + D(10));
            if (v0->sz == 0 || v1->sz == 0 || v2->sz == 0) break;
            if (v2->clip & (v0->clip & v1->clip)) break;
            q = s->cur;
            s->cur = q + 32;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            *(u32 *)(q + 4) = p[3];
            setlen(q, 7);
            setcode(q, s->tag.b.code & 0x7F);
            goto tri;
        case 0xAC:
            v0 = (PVert *)(vt + D(10));
            v1 = (PVert *)(vt + D(11));
            v2 = (PVert *)(vt + D(12));
            v3 = (PVert *)(vt + D(13));
            if (v0->sz == 0 || v1->sz == 0 || v2->sz == 0 || v3->sz == 0) break;
            if (v3->clip & (v2->clip & (v0->clip & v1->clip))) break;
            q = s->cur;
            s->cur = q + 40;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(s32 *)(q + 32) = v3->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            *(u16 *)(q + 36) = D(6);
            *(u32 *)(q + 4) = p[4];
            setlen(q, 9);
            setcode(q, s->tag.b.code & 0x7F);
            goto quad;
        }
        p += s->tag.b.len;
    }
    D_8019D95C.buf->cur = s->cur;
}
