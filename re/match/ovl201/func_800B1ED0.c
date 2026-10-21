#include "ac1/ac1.h"

extern s32 D_80049E74[];

s16 func_800B1ED0(void) {
    s16 n;
    s16 i;

    n = 0;
    for (i = 0; i < 15; i++) {
        if (D_80049E74[i] == 0) {
            n++;
        }
    }
    return n;
}
