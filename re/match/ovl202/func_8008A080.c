#include "ac1/ac1.h"

extern u8 D_80031A94[];

void func_8008A080(s32 i) {
    if (i != 0) {
        D_80031A94[i]++;
    }
}
