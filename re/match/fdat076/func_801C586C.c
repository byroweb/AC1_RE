#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x30];
    u32 flags;          /* +0x30 */
    u8 pad34[0x0C];
} World;
extern World D_801D0B68[];
extern u16 D_8019F528;
extern s16 D_8019F52C;
extern void func_8004C318();
extern void func_800529F0();

void func_801C586C(s32 n)
{
    if (D_8019F528 == 0) {
        if (n < 0x200) {
            D_801D0B68[n].flags = ((D_801D0B68[n].flags & 0x0FFFFFFF) | 0x20000000) & 0xF0FFFFFF;
        }
        if (n == 0x270F) {
            func_800529F0(0, 0, 0x80);
            func_800529F0(0, 0, 0x68);
            D_8019F52C = 0;
            func_8004C318(0x80);
        }
    }
}
