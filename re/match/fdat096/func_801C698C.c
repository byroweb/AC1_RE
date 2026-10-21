#include "ac1/ac1.h"

extern u16 D_8019F528[];
extern void func_8008C2FC(s32, void *);
extern void func_801C68C4();

void func_801C698C(s32 arg0)
{
    if (D_8019F528[0] == 0 && arg0 == 0) {
        func_8008C2FC(769, func_801C68C4);
        D_8019F528[0] = 1;
    }
}
