#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_801C5FD0;

void func_801C6014(void *arg0) {
    if (M2C_FIELD(arg0, u8 *, 0xFE) == 0) {
        M2C_FIELD(arg0, s16 *, 0x36) = 2;
    } else {
        M2C_FIELD(arg0, s16 *, 0x36) = 1;
    }
    M2C_FIELD(arg0, s16 *, 0x32) = 5;
    M2C_FIELD(arg0, M2C_UNK **, 0x58) = &func_801C5FD0;
}
