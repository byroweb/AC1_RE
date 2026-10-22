#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 func_8004F024(void *, void *);
extern void func_80075FC0(void *);
extern void func_80078250(void *, s32, s32, s32, s32, s32, s32, s32);

void func_801C4DD8(void *arg0)
{
    if (M2C_FIELD(arg0, s16 *, 0x2E) <= M2C_FIELD(arg0, s16 *, 0xA)
        && M2C_FIELD(arg0, s16 *, 0x2E) >= M2C_FIELD(arg0, s16 *, 0xA) - M2C_FIELD(arg0, u16 *, 0x2A)) {
        if (M2C_FIELD(arg0, s16 *, 0x6E) == 4) {
            func_80078250(arg0, 9, 1, 0, 0x50, 0xC8, 0x41, 0);
        } else if (M2C_FIELD(arg0, s16 *, 0x6E) == 12) {
            func_80078250(arg0, 4, 6, 0, 0x50, 0xC8, 0x41, 0);
        }
    }
    func_80075FC0(arg0);
    if (func_8004F024((u8 *)arg0 + 0x64, (u8 *)arg0 + 8) == 0) {
        void (*f)(void *, u8) = M2C_FIELD(arg0, void (**)(void *, u8), 0x50);
        if (f != NULL) {
            f(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
