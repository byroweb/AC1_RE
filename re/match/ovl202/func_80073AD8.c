#include "ac1/ac1.h"

typedef struct {
    s16 w;
    s16 h;
    s16 id;
    u8 pad6[0x22];
    u8 *data;
} TexRef;

typedef struct {
    TexRef tex[0x6854 / 0x2C];
    u8 pad[0x6854 % 0x2C];
    u8 *cur;
} TexTbl;

extern TexTbl D_8019F564;
extern u8 *D_801A5DB8; /* D_8019F564.cur, reached by its own name too */
extern void *func_8002A480(void *dst, void *src, s32 n);

void func_80073AD8(TexRef *t, s32 *src, s16 id) {
    s32 n;

    n = *src;
    func_8002A480(D_801A5DB8, src + 1, n);
    t->data = D_8019F564.cur;
    t->w = ((u16 *)t->data)[2] >> 2;
    t->h = ((u16 *)t->data)[3] >> 2;
    t->id = id;
    D_8019F564.cur += ((n + 3) >> 2) * 4;
}
