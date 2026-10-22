#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u8 b[8];
} Func801C5190Bytes8;
extern s16 func_80073904();
extern void func_801C4D30(void *, u8);
extern void func_801C4F60(void *, s32);
extern void func_801C4E58();
extern void func_801C50AC();
extern Func801C5190Bytes8 D_80039C78;

void func_801C5190(void *arg0)
{
    M2C_FIELD(arg0, s16 *, 0x36) = 2;
    M2C_FIELD(arg0, void (**)(void *, u8), 0x50) = func_801C4D30;
    M2C_FIELD(arg0, void (**)(void *, s32), 0x54) = func_801C4F60;
    M2C_FIELD(arg0, void (**)(), 0x58) = func_801C4E58;
    M2C_FIELD(arg0, void (**)(), 0x5C) = func_801C50AC;
    M2C_FIELD(arg0, s8 *, 0x64) = 0;
    M2C_FIELD(arg0, s8 *, 0x42) = 1;
    M2C_FIELD(arg0, u16 *, 0x164) = M2C_FIELD(arg0, u16 *, 0x164) * 4;
    M2C_FIELD(arg0, s16 *, 0x44) = func_80073904();
    if (M2C_FIELD(arg0, void (**)(void *, s32), 0x54) != NULL) {
        M2C_FIELD(arg0, void (**)(void *, s32), 0x54)(arg0, 0);
    }
    if (M2C_FIELD(arg0, void (**)(void *, u8), 0x50) != NULL) {
        M2C_FIELD(arg0, void (**)(void *, u8), 0x50)(arg0, M2C_FIELD(arg0, u8 *, 0x41));
    }
    M2C_FIELD(arg0, Func801C5190Bytes8 *, 0x76) = D_80039C78;
}
