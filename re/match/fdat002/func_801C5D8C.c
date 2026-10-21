#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u8 b[8];
} Func801C5D8CBytes8;
extern s16 func_80073904();
extern void func_801C529C(void *, u8);
extern void func_801C59DC(void *, s32);
extern void func_801C56A8();
extern void func_801C5BFC();
extern Func801C5D8CBytes8 D_80039C78;

void func_801C5D8C(void *arg0)
{
    M2C_FIELD(arg0, void (**)(void *, u8), 0x50) = func_801C529C;
    M2C_FIELD(arg0, void (**)(void *, s32), 0x54) = func_801C59DC;
    M2C_FIELD(arg0, void (**)(), 0x58) = func_801C56A8;
    M2C_FIELD(arg0, void (**)(), 0x5C) = func_801C5BFC;
    M2C_FIELD(arg0, s8 *, 0x64) = 0;
    M2C_FIELD(arg0, s8 *, 0x108) = 0;
    M2C_FIELD(arg0, s8 *, 0x42) = 1;
    M2C_FIELD(arg0, u16 *, 0x164) = M2C_FIELD(arg0, u16 *, 0x164) * 4;
    M2C_FIELD(arg0, s16 *, 0x44) = func_80073904();
    if (M2C_FIELD(arg0, void (**)(void *, s32), 0x54) != NULL) {
        M2C_FIELD(arg0, void (**)(void *, s32), 0x54)(arg0, 0);
    }
    if (M2C_FIELD(arg0, void (**)(void *, u8), 0x50) != NULL) {
        M2C_FIELD(arg0, void (**)(void *, u8), 0x50)(arg0, M2C_FIELD(arg0, u8 *, 0x41));
    }
    M2C_FIELD(arg0, Func801C5D8CBytes8 *, 0x76) = D_80039C78;
}
