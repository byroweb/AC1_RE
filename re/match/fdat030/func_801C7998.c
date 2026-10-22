#include "ac1/ac1.h"

typedef struct {
    u8 st;              /* +0x00 */
    u8 pad1[9];
    s16 kind;           /* +0x0A */
    u8 padC[0x20];
} Slot;
extern Slot D_8019FAB8[];
extern u16 D_8019F528;
extern s32 D_801C7B90;
extern void func_8004C318();
extern void func_800529F0();
extern void func_8008A80C();

void func_801C7998(void)
{
    s32 i;
    s32 n;

    func_8008A80C();
    n = 0;
    if (D_8019F528 == 0) {
        for (i = 4; i < 11; i++) {
            if (D_8019FAB8[i].kind != -1 && D_8019FAB8[i].st == 3) {
                n++;
            }
        }
        if (n >= 4) {
            if (D_801C7B90 == 10) {
                func_800529F0(0, 0, 0x68);
                func_8004C318(0x80);
            }
            D_801C7B90++;
        }
    }
}
