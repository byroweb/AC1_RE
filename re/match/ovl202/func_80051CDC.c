#include "ac1/ac1.h"

extern s16 D_8008DB0C[];
extern void func_800513EC(s32, s32, s32, void *);

void func_80051CDC(s32 arg0, s32 arg1, s32 n) {
    func_800513EC(arg0, arg1, n * 2, D_8008DB0C);
}
