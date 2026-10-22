#include "ac1/ac1.h"
#include "ac1/gte.h"
#include "ac1/prim2d.h"

#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddq(r0) __asm__ volatile ("lw $8, %0\n\tmtc2 $8, $8" : : "m"(*(r0)))
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x10 transformed vertex */
    s32 sxy;                    /* +0x00 */
    s16 sz;                     /* +0x04 */
    u8 pad06[6];
    s32 dq;                     /* +0x0C depth-cue factor */
} DVert;

typedef struct {
    u8 *vert;                   /* +0x00 SVECTOR[], +6 = palette offset */
    u16 n_vert;                 /* +0x04 */
    u8 pad06[2];
    u8 *normals;                /* +0x08 */
    u8 pad0c[4];
    u32 *prim;                  /* +0x10 */
    u16 n_prim;                 /* +0x14 */
} MdlPart;

typedef struct {                /* scratchpad 0x1F800000 */
    s32 clip;                   /* +0x00 */
    u8 *cur;                    /* +0x04 */
    u8 *end;                    /* +0x08 */
    union {
        u32 w;
        struct {
            u8 bias;            /* +0x0C */
            u8 len;             /* +0x0D */
            u8 b2;
            u8 code;            /* +0x0F */
        } b;
    } tag;                      /* +0x0C */
    u8 *normals;                /* +0x10 */
    u8 *vtop;                   /* +0x14 */
    s32 count;                  /* +0x18 */
    u32 *ot;                    /* +0x1C */
    s32 far;                    /* +0x20 */
    s32 shift;                  /* +0x24 */
    u8 *pal;                    /* +0x28 */
    u8 rgb[4];                  /* +0x2C */
} PalScr;

typedef struct {
    PrimBuf *buf;               /* 0x8019D95C */
    u32 *ot;                    /* 0x8019D960 */
    u8 pad08[0x66C];
    u8 verts[1];                /* 0x8019DFD0 */
} RenderCtx;

extern RenderCtx D_8019D95C;
extern s32 D_80194734;
extern u8 D_8008C990[][128];
extern void func_8005AB14(u8 *vert, s32 n);
extern void func_8005B7AC(PalScr *s, s32 a, s32 b, s32 c);
extern void func_8005B7D0(u8 *n, u8 *rgb, s32 dq);

#define D(i) (((u16 *)p)[i])
#define PALRGB(i) gte_ldrgb(s->pal + *(s16 *)(s->vtop + (D(i) >> 1) + 6))

void func_8005ABC0(MdlPart *part, s32 kind, s32 ot)
{
    PalScr *s;
    u8 *vt;
    u32 *p;
    u8 *q;
    DVert *v0;
    DVert *v1;
    DVert *v2;
    DVert *v3;
    s32 n;

    s = (PalScr *)0x1F800000;
    s->count = part->n_prim;
    if (part->n_prim == 0) {
        return;
    }
    s->rgb[0] = s->rgb[1] = s->rgb[2] = 64;
    s->far = D_80194734 + 15;
    s->shift = D_80194734 + 3;
    s->ot = D_8019D95C.ot + ot;
    vt = D_8019D95C.verts;
    func_8005AB14(part->vert, part->n_vert);
    s->vtop = part->vert;
    s->normals = part->normals;
    s->pal = D_8008C990[kind];
    p = part->prim;
    s->cur = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    do {
        s->tag.w = *p++;
        switch (s->tag.b.code & 0xFD) {
        case 0x2C:
            v0 = (DVert *)(vt + D(9));
            if (v0->sz == 0) break;
            v1 = (DVert *)(vt + D(10));
            if (v1->sz == 0) break;
            v2 = (DVert *)(vt + D(11));
            if (v2->sz == 0) break;
            v3 = (DVert *)(vt + D(12));
            if (v3->sz == 0) break;
            func_8005B7AC(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip < 0) break;
            func_8005B7D0(s->normals + D(8), s->rgb, v0->dq);
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
            n = ((v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift) + (s8)s->tag.b.bias * 4;
            if (n >= 0) {
                addPrim(s->ot + n, q);
                gte_strgb(q + 4);
                setcode(q, s->tag.b.code);
            }
            break;
        case 0x24:
            v0 = (DVert *)(vt + D(7));
            if (v0->sz == 0) break;
            v1 = (DVert *)(vt + D(8));
            if (v1->sz == 0) break;
            v2 = (DVert *)(vt + D(9));
            if (v2->sz == 0) break;
            func_8005B7AC(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip < 0) break;
            func_8005B7D0(s->normals + D(6), s->rgb, v0->dq);
            q = s->cur;
            s->cur = q + 32;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            setlen(q, 7);
            n = (((v0->sz + v1->sz + v2->sz) * 0x1555) >> s->far) + (s8)s->tag.b.bias * 4;
            if (n >= 0) {
                addPrim(s->ot + n, q);
                gte_strgb(q + 4);
                setcode(q, s->tag.b.code);
            }
            break;
        case 0x2D:
            v0 = (DVert *)(vt + D(9));
            if (v0->sz == 0) break;
            v1 = (DVert *)(vt + D(10));
            if (v1->sz == 0) break;
            v2 = (DVert *)(vt + D(11));
            if (v2->sz == 0) break;
            v3 = (DVert *)(vt + D(12));
            if (v3->sz == 0) break;
            func_8005B7AC(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip < 0) break;
            gte_ldv0(s->normals + D(8));
            PALRGB(9);
            gte_lddq(&v0->dq);
            gte_ncds();
            q = s->cur;
            s->cur = q + 52;
            gte_strgb(q + 4);
            PALRGB(10);
            gte_lddq(&v1->dq);
            gte_ncds();
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 20) = v1->sxy;
            *(s32 *)(q + 32) = v2->sxy;
            *(s32 *)(q + 44) = v3->sxy;
            gte_strgb(q + 16);
            PALRGB(11);
            gte_lddq(&v2->dq);
            gte_ncds();
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 24) = p[1];
            *(u16 *)(q + 36) = D(4);
            *(u16 *)(q + 48) = D(6);
            gte_strgb(q + 28);
            PALRGB(12);
            gte_lddq(&v3->dq);
            gte_ncds();
            setlen(q, 12);
            setcode(q, (s->tag.b.code & 2) | 0x3C);
            n = ((v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift) + (s8)s->tag.b.bias * 4;
            if (n >= 0) {
                addPrim(s->ot + n, q);
                gte_strgb(q + 40);
            }
            break;
        case 0x25:
            v0 = (DVert *)(vt + D(7));
            if (v0->sz == 0) break;
            v1 = (DVert *)(vt + D(8));
            if (v1->sz == 0) break;
            v2 = (DVert *)(vt + D(9));
            if (v2->sz == 0) break;
            func_8005B7AC(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip < 0) break;
            gte_ldv0(s->normals + D(6));
            PALRGB(7);
            gte_lddq(&v0->dq);
            gte_ncds();
            q = s->cur;
            s->cur = q + 40;
            gte_strgb(q + 4);
            PALRGB(8);
            gte_lddq(&v1->dq);
            gte_ncds();
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 20) = v1->sxy;
            *(s32 *)(q + 32) = v2->sxy;
            gte_strgb(q + 16);
            PALRGB(9);
            gte_lddq(&v2->dq);
            gte_ncds();
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 24) = p[1];
            *(u16 *)(q + 36) = D(4);
            setlen(q, 9);
            setcode(q, (s->tag.b.code & 2) | 0x34);
            n = (((v0->sz + v1->sz + v2->sz) * 0x1555) >> s->far) + (s8)s->tag.b.bias * 4;
            if (n >= 0) {
                addPrim(s->ot + n, q);
                gte_strgb(q + 28);
            }
            break;
        case 0x6C:
            v0 = (DVert *)(vt + D(9));
            if (v0->sz == 0) break;
            func_8005B7D0(s->normals + D(8), s->rgb, v0->dq);
            q = s->cur;
            s->cur = q + 40;
            v1 = (DVert *)(vt + D(10));
            if (v1->sz == 0) break;
            v2 = (DVert *)(vt + D(11));
            if (v2->sz == 0) break;
            v3 = (DVert *)(vt + D(12));
            if (v3->sz == 0) break;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(s32 *)(q + 32) = v3->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = D(4);
            *(u16 *)(q + 36) = D(6);
            setlen(q, 9);
            n = ((v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift) + (s8)s->tag.b.bias * 4;
            if (n >= 0) {
                addPrim(s->ot + n, q);
                gte_strgb(q + 4);
                setcode(q, s->tag.b.code & 0xBF);
            }
            break;
        }
        p += s->tag.b.len;
    } while (--s->count != 0);
    D_8019D95C.buf->cur = s->cur;
}
