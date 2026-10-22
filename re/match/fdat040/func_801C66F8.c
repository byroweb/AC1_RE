#include "ac1/ac1.h"

typedef struct { s32 w; } __attribute__((packed)) U;
extern void func_800529F0();
extern void func_8008A778();
extern void func_801C66D0();
extern U D_8008DCC4[];
extern U D_801CCB6C[];
extern u16 D_8019F52C;
extern s8 D_8019FB68;
extern s8 D_8019FB94;
extern s8 D_8019FC9C;
extern s8 D_8019FD78;
extern s32 D_801CCB60;
extern s32 D_801CCB64;

s32 func_801C66F8(s32 a0)
{
    U *s;
    U *d;
    s32 n;
    u16 *q;

    switch (a0) {
    case 0:
        q = &D_8019F52C;
        if (*q != 0) {
            *q = 0;
            D_801CCB60 = 1;
            return 1;
        }
        break;
    case 1:
        n = 8;
        s = D_801CCB6C;
        d = D_8008DCC4;
        do {
            *d++ = *s++;
        } while (--n != 0);
        func_801C66D0(0xFB);
        func_8008A778(0xA50, 0x100, 0x79);
        func_800529F0(0, 0, 0xB);
        func_800529F0(0, 0, 0xC);
        D_8019FB68 = 3;
        D_8019FB94 = 3;
        D_8019FC9C = 3;
        D_8019FD78 = 3;
        D_801CCB64 = 1;
        break;
    }
    return 0;
}
