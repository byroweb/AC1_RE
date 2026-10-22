#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80075418(void *, s32, s32, s32, s32);
typedef struct {
    u8 pad0[2];
    u8 kind;
    u8 pad3[11];
} Func801C66C4Ent;
extern Func801C66C4Ent D_801C7540[];

void func_801C66C4(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    if ((D_801C7540[M2C_FIELD(arg0, u8 *, 0x42)].kind == 9) && (M2C_FIELD(arg0, u8 *, 0x10A) == M2C_FIELD(arg0, u8 *, 0x10B)) && (M2C_FIELD(arg0, u16 *, 0x160) == 0)) {
        func_80075418(arg0, arg1, arg4, 0xC8, -1);
        M2C_FIELD(arg0, s8 *, 0x3C) = 0;
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) | 0x10);
    }
}
