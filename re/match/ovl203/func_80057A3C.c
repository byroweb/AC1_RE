#include "ac1/ac1.h"
#include "ac1/gte.h"
#include "ac1/prim2d.h"

#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddq(r0) __asm__ volatile ("lw $8, %0\n\tmtc2 $8, $8" : : "m"(*(r0)))
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_ncdt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AF80416")
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")
#define gte_strgb3(r0, r1, r2) __asm__ volatile ("swc2 $20, 0(%0)\n\tswc2 $21, 0(%1)\n\tswc2 $22, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")

typedef struct {                /* 0x10 transformed vertex */
    s32 sxy;                    /* +0x00 */
    s16 sz;                     /* +0x04 */
    u8 pad06[6];
    s32 dq;                     /* +0x0C depth-cue factor */
} DVert;

typedef struct {                /* 0x1C */
    u32 *vert;                  /* +0x00 */
    u16 n_vert;                 /* +0x04 */
    u16 n_vert2;                /* +0x06 */
    u8 *normals;                /* +0x08 */
    u8 pad0c[2];
    u16 n_extra;                /* +0x0E (& 0x1FF) */
    u32 *prim;                  /* +0x10 */
    u16 n_prim;                 /* +0x14 */
    u8 pad16[6];
} MdlObj;

typedef struct {                /* scratchpad 0x1F800000 */
    s32 clip;                   /* +0x00 */
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
    u8 *normals;                /* +0x14 */
    s32 count;                  /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 far;                    /* +0x20 */
    s32 shift;                  /* +0x24 */
    s32 zmin;                   /* +0x28 */
    u8 rgb[4];                  /* +0x2C */
} NcdScr;

typedef struct {
    PrimBuf *buf;               /* 0x8019D95C */
    u32 *ot;                    /* 0x8019D960 */
    u8 pad08[4];
    u8 *model;                  /* 0x8019D968 */
    u8 pad10[0x664];
    u8 verts[1];                /* 0x8019DFD0 */
} RenderCtx;

extern RenderCtx D_8019D95C;
extern s32 D_80194734;
extern void func_80057964(s32 n);
extern void func_80057A18(NcdScr *s, s32 a, s32 b, s32 c);

#define D(i) (((u16 *)p)[i])

void func_80057A3C(u16 idx, s32 ot, s32 full)
{
    NcdScr *s;
    MdlObj *obj;
    u8 *vt;
    u32 *p;
    u8 *q;
    DVert *v0;
    DVert *v1;
    DVert *v2;
    DVert *v3;          /* shared by the 0x2C/0x3C quads */
    DVert *v3_AC;
    s32 z;

    s = (NcdScr *)0x1F800000;
    s->rgb[0] = s->rgb[1] = s->rgb[2] = 64;
    gte_ldrgb(s->rgb);
    s->zmin = 0;
    s->far = D_80194734 + 15;
    s->shift = D_80194734 + 3;
    s->ot = D_8019D95C.ot + ot;
    obj = (MdlObj *)(D_8019D95C.model + (idx * 32 + 12) - idx * 4);
    s->obj = obj;
    vt = D_8019D95C.verts;
    if (obj->n_vert == 0) {
        return;
    }
    if (full != 0) {
        func_80057964(obj->n_vert + obj->n_vert2);
        s->count = s->obj->n_prim + (s->obj->n_extra & 0x1FF);
    } else {
        func_80057964(obj->n_vert);
        s->count = s->obj->n_prim;
    }
    s->normals = s->obj->normals;
    p = s->obj->prim;
    s->cur = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    if (s->end < s->cur + (s->count << 2) + (s->count << 4) + (s->count << 5)) {
        return;
    }
    while (s->count-- != 0) {
        s->tag.w = *p++;
        switch (s->tag.b.code & 0xFC) {
        case 0x24:
            v0 = (DVert *)(vt + D(7));
            v1 = (DVert *)(vt + D(8));
            v2 = (DVert *)(vt + D(9));
            func_80057A18(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip <= 0) break;
            gte_ldv0(s->normals + D(6));
            gte_lddq(&v0->dq);
            gte_ncds();
            q = s->cur;
            s->cur = q + 32;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            setlen(q, 7);
            z = ((v0->sz + v1->sz + v2->sz) * 0x1555) >> s->far;
            if (s->zmin < z) {
                addPrim(s->ot + z, q);
                gte_strgb(q + 4);
                setcode(q, s->tag.b.code & 0xFE);
            }
            break;
        case 0x2C:
            v0 = (DVert *)(vt + D(9));
            v1 = (DVert *)(vt + D(10));
            v2 = (DVert *)(vt + D(11));
            func_80057A18(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip <= 0) break;
            gte_ldv0(s->normals + D(8));
            gte_lddq(&v0->dq);
            gte_ncds();
            q = s->cur;
            v3 = (DVert *)(vt + D(12));
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
            z = (v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift;
            if (s->zmin < z) {
                addPrim(s->ot + z, q);
                gte_strgb(q + 4);
                setcode(q, s->tag.b.code & 0xFE);
            }
            break;
        case 0x34:
            v0 = (DVert *)(vt + D(7));
            v1 = (DVert *)(vt + D(9));
            v2 = (DVert *)(vt + D(11));
            func_80057A18(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip <= 0) break;
            gte_ldv3(s->normals + D(6), s->normals + D(8), s->normals + D(10));
            gte_lddq(&v0->dq);
            gte_ncdt();
            q = s->cur;
            s->cur = q + 40;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 20) = v1->sxy;
            *(s32 *)(q + 32) = v2->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 24) = p[1];
            *(u16 *)(q + 36) = D(4);
            setlen(q, 9);
            z = ((v0->sz + v1->sz + v2->sz) * 0x1555) >> s->far;
            if (s->zmin < z) {
                addPrim(s->ot + z, q);
                gte_strgb3(q + 4, q + 16, q + 28);
                setcode(q, s->tag.b.code & 0xFE);
            }
            break;
        case 0x3C:
            v0 = (DVert *)(vt + D(9));
            v1 = (DVert *)(vt + D(11));
            v2 = (DVert *)(vt + D(13));
            func_80057A18(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip <= 0) break;
            gte_ldv3(s->normals + D(8), s->normals + D(10), s->normals + D(12));
            gte_lddq(&v0->dq);
            gte_ncdt();
            q = s->cur;
            v3 = (DVert *)(vt + D(15));
            s->cur = q + 52;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 20) = v1->sxy;
            *(s32 *)(q + 32) = v2->sxy;
            *(s32 *)(q + 44) = v3->sxy;
            gte_strgb3(q + 4, q + 16, q + 28);
            gte_ldv0(s->normals + D(14));
            gte_lddq(&v0->dq);
            gte_ncds();
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 24) = p[1];
            *(u16 *)(q + 36) = D(4);
            *(u16 *)(q + 48) = D(6);
            setlen(q, 12);
            setcode(q, s->tag.b.code & 0xFE);
            z = (v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift;
            if (s->zmin < z) {
                addPrim(s->ot + z, q);
                gte_strgb(q + 40);
            }
            break;
        case 0xA4:
            v0 = (DVert *)(vt + D(8));
            q = s->cur;
            v1 = (DVert *)(vt + D(9));
            v2 = (DVert *)(vt + D(10));
            s->cur = q + 32;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(u32 *)(q + 4) = p[3];
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            setlen(q, 7);
            setcode(q, s->tag.b.code & 0x7F);
            z = ((v0->sz + v1->sz + v2->sz) * 0x1555) >> s->far;
            if (s->zmin < z) {
                addPrim(s->ot + z, q);
            }
            break;
        case 0xAC:
            v0 = (DVert *)(vt + D(10));
            q = s->cur;
            v1 = (DVert *)(vt + D(11));
            v2 = (DVert *)(vt + D(12));
            v3_AC = (DVert *)(vt + D(13));
            s->cur = q + 40;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(s32 *)(q + 32) = v3_AC->sxy;
            *(u32 *)(q + 4) = p[4];
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            *(u16 *)(q + 36) = D(6);
            setlen(q, 9);
            setcode(q, s->tag.b.code & 0x7F);
            z = (v0->sz + v1->sz + v2->sz + v3_AC->sz) >> s->shift;
            if (s->zmin < z) {
                addPrim(s->ot + z, q);
            }
            break;
        }
        p += s->tag.b.len;
    }
    D_8019D95C.buf->cur = s->cur;
}
