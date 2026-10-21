#include "ac1/ac1.h"
#include "ac1/ovl201_t49.h"

extern void func_80096424();
extern s32 func_800517C8(void);

void func_8009646C(void) {
    s32 i;
    s32 k = 64;
    s32 j = 64;

    D_8017F6D0.f790 = 0;
    D_8017F6D0.f791 = 0;
    for (i = 63; i != -1; i--) {
        D_8017F6D0.a6d0[i] = 0;
    }
    while (--j != -1) {
        D_8017F6D0.cur[j].a = -1;
        D_8017F6D0.cur[j].b = -1;
    }
    D_8017F6D0.f794 = 0;
    D_8017F6D0.f795 = 0;
    D_8017F6D0.f796 = 0;
    while (--k != -1) {
        D_8017F6D0.a7a0[k] = -1;
    }
    func_80096424();
    while ((s16)func_800517C8() != 0) {
    }
}
