#include "ac1/ac1.h"
#include "ac1/prim2d.h"

#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x10 transformed vertex */
    s32 sxy;                    /* +0x00 */
    s16 sz;                     /* +0x04 */
    u8 pad06[2];
    u8 n[3];                    /* +0x08 normal/colour input */
    u8 clip;                    /* +0x0B */
    s32 c;                      /* +0x0C depth cue */
} LVert;

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
    union {
        u32 w;
        struct {
            u8 bias;            /* +0x0C */
            u8 len;             /* +0x0D */
            u8 b2;
            u8 code;            /* +0x0F */
        } b;
    } tag;
    MdlObj *obj;                /* +0x10 */
    u8 *normals;                /* +0x14 */
    u32 *vtop;                  /* +0x18 */
    s32 count;                  /* +0x1C */
    u32 *ot;                    /* +0x20 */
    s32 far;                    /* +0x24 */
    s32 shift;                  /* +0x28 */
    u8 rgb[4];                  /* +0x2C */
} LitScr;

typedef struct {
    u8 *model;                  /* 0x8019D968 */
    u8 pad04[0x664];
    u8 verts[1];                /* 0x8019DFD0 LVert by byte offset */
} RenderGlobals;

extern RenderGlobals D_8019D968;
extern DrawCtx D_8019D95C;
extern u32 *D_8019DF7C;
extern s32 D_80194734;
extern void func_8005AB14(u32 *vert, s32 n);
extern void func_8005B7AC(LitScr *s, s32 a, s32 b, s32 c);
extern void func_8005B7D0(u8 *n, u8 *rgb, s32 c);
extern void func_8005B7F4(u8 *rgb, s32 c);

void func_8005B810(u16 idx, s32 ot)
{
    LitScr *s;
    MdlObj *obj;
    u8 *vt;
    u32 *p;
    u8 *q;
    LVert *v0;
    LVert *v1;
    LVert *v2;
    LVert *v3;
    s32 z;
    s32 bias;
    s32 n;

    s = (LitScr *)0x1F800000;
    obj = (MdlObj *)(D_8019D968.model + (idx * 32 + 12) - idx * 4);
    s->obj = obj;
    if (obj->n_prim == 0) {
        return;
    }
    s->count = obj->n_prim;
    s->rgb[0] = s->rgb[1] = s->rgb[2] = 64;
    s->far = D_80194734 + 15;
    s->shift = D_80194734 + 3;
    s->ot = D_8019D95C.ot + ot;
    vt = D_8019D968.verts;
    func_8005AB14(obj->vert, obj->n_vert);
    s->normals = s->obj->normals;
    s->vtop = D_8019DF7C;
    p = s->obj->prim;
    s->cur = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    do {
        s->tag.w = *p++;
        switch (s->tag.b.code & 0xFC) {
        case 0x2C:
            v0 = (LVert *)(vt + ((u16 *)p)[9]);
            if (v0->sz == 0) break;
            v1 = (LVert *)(vt + ((u16 *)p)[10]);
            if (v1->sz == 0) break;
            v2 = (LVert *)(vt + ((u16 *)p)[11]);
            if (v2->sz == 0) break;
            v3 = (LVert *)(vt + ((u16 *)p)[12]);
            if (v3->sz == 0) break;
            func_8005B7AC(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip < 0) break;
            if ((v3->clip | (v2->clip | (v0->clip | v1->clip))) == 0) {
                func_8005B7D0(s->normals + ((u16 *)p)[8], s->rgb, v0->c);
                q = s->cur;
                s->cur = q + 40;
                *(s32 *)(q + 8) = v0->sxy;
                *(s32 *)(q + 16) = v1->sxy;
                *(s32 *)(q + 24) = v2->sxy;
                *(s32 *)(q + 32) = v3->sxy;
                *(u32 *)(q + 12) = p[0];
                *(u32 *)(q + 20) = p[1];
                *(u16 *)(q + 28) = ((u16 *)p)[4];
                *(u16 *)(q + 36) = ((u16 *)p)[6];
                q[3] = 9;
                gte_strgb(q + 4);
                q[7] = s->tag.b.code & 0xFE;
            } else {
            gt4:
                func_8005B7D0(s->normals + ((u16 *)p)[8], v0->n, v0->c);
                q = s->cur;
                s->cur = q + 52;
                gte_strgb(q + 4);
                func_8005B7F4(v1->n, v1->c);
                *(s32 *)(q + 8) = v0->sxy;
                *(s32 *)(q + 20) = v1->sxy;
                *(s32 *)(q + 32) = v2->sxy;
                *(s32 *)(q + 44) = v3->sxy;
                gte_strgb(q + 16);
                func_8005B7F4(v2->n, v2->c);
                *(u32 *)(q + 12) = p[0];
                *(u32 *)(q + 24) = p[1];
                *(u16 *)(q + 36) = ((u16 *)p)[4];
                *(u16 *)(q + 48) = ((u16 *)p)[6];
                gte_strgb(q + 28);
                func_8005B7F4(v3->n, v3->c);
                q[3] = 12;
                q[7] = (s->tag.b.code & 2) | 0x3C;
                gte_strgb(q + 40);
            }
        z4:
            bias = (v0->sz + v1->sz + v2->sz + v3->sz) >> s->shift;
            z = (s8)s->tag.b.bias * 4;
            n = z + bias;
            if (n >= 0) {
                addPrim(s->ot + n, q);
            }
            break;
        case 0x24:
            v0 = (LVert *)(vt + ((u16 *)p)[7]);
            if (v0->sz == 0) break;
            v1 = (LVert *)(vt + ((u16 *)p)[8]);
            if (v1->sz == 0) break;
            v2 = (LVert *)(vt + ((u16 *)p)[9]);
            if (v2->sz == 0) break;
            func_8005B7AC(s, v0->sxy, v1->sxy, v2->sxy);
            if (s->clip < 0) break;
            if ((v2->clip | (v0->clip | v1->clip)) == 0) {
                func_8005B7D0(s->normals + ((u16 *)p)[6], s->rgb, v0->c);
                q = s->cur;
                s->cur = q + 32;
                *(s32 *)(q + 8) = v0->sxy;
                *(s32 *)(q + 16) = v1->sxy;
                *(s32 *)(q + 24) = v2->sxy;
                *(u32 *)(q + 12) = p[0];
                *(u32 *)(q + 20) = p[1];
                *(u16 *)(q + 28) = ((u16 *)p)[4];
                q[3] = 7;
                gte_strgb(q + 4);
                q[7] = s->tag.b.code & 0xFE;
            } else {
                func_8005B7D0(s->normals + ((u16 *)p)[6], v0->n, v0->c);
                q = s->cur;
                s->cur = q + 40;
                gte_strgb(q + 4);
                func_8005B7F4(v1->n, v1->c);
                *(s32 *)(q + 8) = v0->sxy;
                *(s32 *)(q + 20) = v1->sxy;
                *(s32 *)(q + 32) = v2->sxy;
                gte_strgb(q + 16);
                func_8005B7F4(v2->n, v2->c);
                *(u32 *)(q + 12) = p[0];
                *(u32 *)(q + 24) = p[1];
                *(u16 *)(q + 36) = ((u16 *)p)[4];
                q[3] = 9;
                q[7] = (s->tag.b.code & 2) | 0x34;
                gte_strgb(q + 28);
            }
            z = ((v0->sz + v1->sz + v2->sz) * 0x1555) >> s->far;
            bias = (s8)s->tag.b.bias * 4;
            n = z + bias;
            if (n >= 0) {
                addPrim(s->ot + n, q);
            }
            break;
        case 0x6C:
            v0 = (LVert *)(vt + ((u16 *)p)[9]);
            if (v0->sz == 0) break;
            v1 = (LVert *)(vt + ((u16 *)p)[10]);
            if (v1->sz == 0) break;
            v2 = (LVert *)(vt + ((u16 *)p)[11]);
            if (v2->sz == 0) break;
            v3 = (LVert *)(vt + ((u16 *)p)[12]);
            if (v3->sz == 0) break;
            if ((v3->clip | (v2->clip | (v0->clip | v1->clip))) != 0) {
                goto gt4;
            }
            func_8005B7D0(s->normals + ((u16 *)p)[8], s->rgb, v0->c);
            q = s->cur;
            s->cur = q + 40;
            *(s32 *)(q + 8) = v0->sxy;
            *(s32 *)(q + 16) = v1->sxy;
            *(s32 *)(q + 24) = v2->sxy;
            *(s32 *)(q + 32) = v3->sxy;
            *(u32 *)(q + 12) = p[0];
            *(u32 *)(q + 20) = p[1];
            *(u16 *)(q + 28) = ((u16 *)p)[4];
            *(u16 *)(q + 36) = ((u16 *)p)[6];
            q[3] = 9;
            gte_strgb(q + 4);
            q[7] = s->tag.b.code & 0xBE;
            goto z4;
        }
        p += s->tag.b.len;
    } while (--s->count != 0);
    D_8019D95C.buf->cur = s->cur;
}
