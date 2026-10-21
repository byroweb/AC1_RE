#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800733D8();
M2C_UNK func_80074620(s32, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */

void func_8006D5BC(s32 arg0, M2C_UNK arg1, M2C_UNK arg2, M2C_UNK arg3) {
    if (func_800733D8(arg0, arg1, arg2, arg3, 0) == 0) {
        func_80074620(arg0, arg1, arg2, arg3, 0);
    }
}
