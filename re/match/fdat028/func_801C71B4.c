#include "ac1/ac1.h"

extern void func_8008A80C();
extern void func_801C6FE4(void *);
extern void func_801C70CC(void *);
extern u16 D_801987D8;
extern s32 D_8019F524;
extern char D_801C7494[], D_801C749C[], D_801C74A4[], D_801C74AC[], D_801C74B4[];

void func_801C71B4(void)
{
    func_8008A80C();
    func_801C6FE4(D_801C7494);
    func_801C70CC(D_801C749C);
    func_801C70CC(D_801C74A4);
    func_801C70CC(D_801C74AC);
    func_801C70CC(D_801C74B4);
    if (D_8019F524 & 1) {
        u16 *p = &D_801987D8;
        *p |= 1;
    }
}
