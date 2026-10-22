#include "ac1/ac1.h"

extern void func_800775B8(AcEntity *e, s32 a, s32 b, s32 c, s32 d, s32 f);

void func_800778B8(AcEntity *e, s32 a, s32 c, s32 d, s32 v, s32 f)
{
    func_800775B8(e, a, 0, c, d, f);
    if (v >= 0) {
        *(s16 *)((u8 *)e + 0xD2) = v;
    }
}
