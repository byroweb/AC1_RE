#include "ac1/ac1.h"

extern s32 func_800B5404(void);
extern u32 func_800B549C(void);

s32 func_800B50C8(s32 sync)
{
    s32 r;

    if (sync == 0) {
        r = func_800B5404();
    } else {
        r = (func_800B549C() >> 24) & 1;
    }
    return r;
}
