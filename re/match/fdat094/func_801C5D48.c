#include "ac1/ac1.h"

typedef struct {
    u8 st;
    u8 pad1[9];
    s16 kind;           /* +0x0A */
    u8 padC[8];
    u16 flags;          /* +0x14 */
    u8 pad16[0x16];
} Slot;
extern Slot D_8019FB68[];
extern void (**D_8019F51C)();
extern void func_801C52C0();
extern void func_801C5314();
extern void func_800529F0();
extern void func_80078CFC();
extern u16 D_8019F528;
extern s32 D_801CCEB8;

s32 func_801C5D48(s32 a0)
{
    u16 *s;
    u16 fl;
    s32 n;
    void (*f)();
    void (*fa)();
    void (*fb)();

    if (D_8019F528 == 0) {
        switch (a0) {
        case 0:
            if (D_801CCEB8 == 0) {
                n = 0xFC;
                fa = func_801C52C0;
                fb = func_801C5314;
                s = &D_8019FB68[0].flags;
                do {
                    fl = *s;
                    if (fl & 0x40) {
                        f = D_8019F51C[((s16 *)s)[-5] + 8];
                        if (f == fa || f == fb) {
                            *s = fl & 0xFFBF;
                        }
                    }
                    n--;
                    s = (u16 *)((u8 *)s + 0x2C);
                } while (n != 0);
                func_800529F0(0, 0, 0x5F);
                D_801CCEB8 = 1;
            }
            break;
        case 1:
            if (D_801CCEB8 == 0) {
                return 0;
            }
            func_800529F0(0, 0, 0x40);
            func_800529F0(0, 0, 0x41);
            func_80078CFC(D_8019FB68);
            break;
        }
    }
    return 1;
}
