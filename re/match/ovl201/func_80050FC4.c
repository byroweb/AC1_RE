#include "ac1_types.h"
extern volatile s32 D_800CF378;
extern void func_80029B40(void);
extern void *func_8002A1A4(u32);
extern void func_80029B50(void);
void *func_80050FC4(u32 n)
{
    u32 size = (n + 3) & ~3;
    void *p;
retry:
    func_80029B40();
    p = func_8002A1A4(size);
    func_80029B50();
    if (p) {
        return p;
    }
    for (D_800CF378 = 0; D_800CF378 < 32767; D_800CF378++) {
    }
    goto retry;
}
