#include "ac1/ac1.h"

typedef struct { s32 w; } __attribute__((packed)) U;
typedef struct {
    u8 pad0[0x1E];
    s16 mesh;           /* +0x1E */
    u8 pad20[0xC];
    void *on_hit;       /* +0x2C */
    u8 pad30[0x10];
} World;
extern void func_800529F0();
extern void func_8004D178();
extern void func_80062860();
extern void func_8008A80C();
extern s32 rsin(s32);
extern U D_8008DCC4[];
extern U D_801C79DC[];
extern World D_801D0B68[];
extern s32 D_801AC81C;
extern s32 D_801C79FC;
extern s32 D_801C7A00;
extern s32 D_801E8B6C;

void func_801C7630(void)
{
    World *w;
    s32 n;
    U *d;
    U *s;
    s32 k;

    func_8008A80C();
    if (D_801C7A00 == 0) {
        n = D_801E8B6C;
        w = D_801D0B68;
        do {
            if (w->mesh == 0x47 && w->on_hit == 0) {
                k = 8;
                s = D_801C79DC;
                d = D_8008DCC4;
                do {
                    *d++ = *s++;
                } while (--k != 0);
                func_800529F0(0, 0, 0x29);
                func_800529F0(0, 0, 0x2A);
                D_801C79FC = 1;
                D_801C7A00 = 1;
                return;
            }
            w++;
        } while (--n != 0);
    } else {
        func_80062860(0, 9, 8, (rsin(D_801AC81C << 9) + 0x1400) >> 1);
        func_8004D178(0x4B, 0x6000, 0x64, 0x64, 0);
    }
}
