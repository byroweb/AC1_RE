#include "ac1/ac1.h"

typedef struct {
    u8 st;
    u8 pad1[0x2B];
} Slot;
typedef struct {
    s32 f0;
    u8 pad1[0x3C];
    u8 f40;
    u8 pad41[0x12F];
} Ent;
typedef struct {
    u8 pad0[4];
    s32 (*fn)(void *, s32, s32);
} Vt;
extern void func_800529F0();
extern void func_80078CFC();
extern void func_801C6C5C();
extern u16 D_8019F528;
extern Slot D_8019FC44[];
extern Ent D_801A2828[];
extern s32 D_801CCA80;
extern u8 D_801D23A8[];
extern Vt *D_801D23D4;

s32 func_801C6D24(s32 a0)
{
    Ent *e;
    s32 n;

    if (D_8019F528 == 0) {
        switch (a0) {
        case 1:
            n = 0xF;
            e = D_801A2828;
            do {
                if (e->f0 != 0) {
                    e->f40 = 0;
                }
                n--;
                e++;
            } while (n != 0);
            func_800529F0(0, 0, 9);
            D_801CCA80 = 1;
            break;
        case 2:
            func_800529F0(0, 0, 0x71);
            func_80078CFC(&D_8019FC44[0]);
            func_80078CFC(&D_8019FC44[1]);
            func_801C6C5C(0x1F40, 0xB, 4, 1);
            break;
        case 3:
            func_800529F0(0, 0, 0x72);
            func_801C6C5C(0x1F40, 0xF, 6, 3);
            if (D_801D23D4 != 0) {
                if (D_801D23D4->fn != 0) {
                    D_801D23D4->fn(D_801D23A8, 0x8200, 0);
                }
            }
            break;
        }
    }
    return 1;
}
