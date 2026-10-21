#include "ac1/ac1.h"

extern void func_80015538(void *p, s32 v, s32 n);
extern void func_8007398C(s32);
extern void func_8007399C(s32);
extern void *D_801A5DB8;
extern s32 D_801A5DC4;
extern s32 D_801A5DC8;

void func_80073BC8(void) {
    AcEntity *e = g_ac_entities;

    func_80015538(e, 0, 0x5C0);
    D_801A5DB8 = e + 16;
    D_801A5DC4 = 0;
    func_8007398C(0);
    D_801A5DC8 = 0;
    func_8007399C(0);
}
