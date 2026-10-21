#include "ac1/ac1.h"

typedef struct {                /* 0x14 */
    s32 id;                     /* +0x00 */
    s32 a;                      /* +0x04 */
    s32 b;                      /* +0x08 */
    u16 c;                      /* +0x0C */
    s16 d;                      /* +0x0E */
    u16 e;                      /* +0x10 */
    u16 pad12;
} PrimSub;

typedef struct {                /* 0x20 */
    u32 *src;                   /* +0x00 */
    u8 v[8];                    /* +0x04 */
    PrimSub sub;                /* +0x0C */
} PrimOut;

typedef struct {                /* 0x1C */
    u32 f00;
    u8 pad04[0x0C];
    u32 *list;                  /* +0x10 */
    u16 count;                  /* +0x14 */
    u8 pad16[6];
} PrimElem;

typedef struct {                /* scratchpad 0x1F800000 */
    u8 pad00[0x54];
    PrimElem *elems;            /* +0x54 */
    union {
        u32 w;                  /* +0x58 copy of the primitive tag */
        struct {
            u8 b0;
            u8 len;             /* +0x59 words after the tag */
            u8 b2;
            u8 code;            /* +0x5B GPU command */
        } b;
    } tag;
    u32 f5c;                    /* +0x5C */
    u8 pad60[0x24];
    s32 counter;                /* +0x84 */
    PrimOut *out;               /* +0x88 */
} PrimSpad;

#define SPAD ((PrimSpad *)0x1F800000)

extern void func_80055EB8(PrimSpad *, u8 *, u16, u16, u16);
extern void func_80055FD8(PrimSpad *, u8 *, u16, u16, u16, u16);

void func_800560F4(s32 idx) {
    PrimOut *out;
    PrimElem *e;
    u32 *p;
    s32 n;
    u16 *h;
    PrimSpad *sp;

    sp = SPAD;
    out = sp->out;
    e = &sp->elems[idx];
    sp->f5c = e->f00;
    n = e->count;
    p = e->list;
    while (--n != -1) {
    next:
        sp->tag.w = *p;
        if (out->sub.id == sp->counter) {
            out->src = p;
            h = (u16 *)p;
            switch (sp->tag.b.code & 0xBC) {
            case 0x24:
                func_80055EB8(sp, out->v, h[9], h[10], h[11]);
                out->sub.a = p[1];
                out->sub.b = p[2];
                out->sub.c = h[6];
                out->sub.d = 1;
                break;
            case 0x2C:
                func_80055FD8(sp, out->v, h[11], h[12], h[13], h[14]);
                out->sub.a = p[1];
                out->sub.b = p[2];
                out->sub.c = h[6];
                out->sub.e = h[8];
                out->sub.d = 0;
                break;
            case 0x34:
                func_80055EB8(sp, out->v, h[9], h[11], h[13]);
                out->sub.a = p[1];
                out->sub.b = p[2];
                out->sub.c = h[6];
                out->sub.d = 1;
                break;
            case 0x3C:
                func_80055FD8(sp, out->v, h[11], h[13], h[15], h[17]);
                out->sub.a = p[1];
                out->sub.b = p[2];
                out->sub.c = h[6];
                out->sub.e = h[8];
                out->sub.d = 0;
                break;
            default:
                out->sub.id = -1;
                break;
            }
            out++;
            goto next;
        }
        p += sp->tag.b.len + 1;
        sp->counter++;
    }
    sp->out = out;
}
