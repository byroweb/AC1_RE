#include "ac1/ac1.h"

extern s32 func_800B536C(void);
extern u32 func_800B549C(void);

s32 func_800B508C(s32 sync)
{
    s32 r;

    if (sync == 0) {
        r = func_800B536C();
    } else {
        r = (func_800B549C() >> 29) & 1;
    }
    return r;
}
