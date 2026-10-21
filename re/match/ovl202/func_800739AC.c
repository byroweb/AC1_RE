/* ovl202 0x800739AC  size 300  permuter tier (tools/loop/permute.py): levers swap func_800154D8(t->hdr, sr / rest = size - 0x24; + swap t = D_8019F564.tex; / p = arg0;, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"

typedef struct {
    s16 w;
    s16 h;
    u8 hdr[0x24];
    u8 *data;
} TexRef;

typedef struct {
    TexRef tex[0x6854 / 0x2C];
    u8 pad[0x6854 % 0x2C];
    u8 *cur;
} TexTbl;

extern TexTbl D_8019F564;
extern u8 *D_801A5DB8; /* D_8019F564.cur, reached by its own name too */
extern void func_800154D8(void *dst, void *src, s32 nwords);
extern void *func_8002A480(void *dst, void *src, s32 n);

void func_800739AC(s32 *arg0) {
    TexRef *t;
    s32 *p;
    u8 *src;
    s32 size;
    s32 rest;
    u16 n;

    p = arg0;
    t = D_8019F564.tex;
    n = *p++;
    while (n--) {
        src = (u8 *)(p + 1);
        size = *p;
        p = (s32 *)((u8 *)p + (size + 4));
        rest = size - 0x24;
        func_800154D8(t->hdr, src, 9);
        if (rest != 0) {
            func_8002A480(D_801A5DB8, src + 0x24, rest);
            t->data = D_8019F564.cur;
            t->w = ((u16 *)(src + 0x24))[2] >> 2;
            t->h = ((u16 *)(src + 0x24))[3] >> 2;
            D_8019F564.cur += ((rest + 3) >> 2) * 4;
        } else {
            t->data = 0;
            t->w = 0;
            t->h = 0;
        }
        t++;
    }
}
