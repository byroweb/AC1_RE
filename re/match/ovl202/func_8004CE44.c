#include "ac1/ac1.h"

extern void func_8004CC30(s32 id, void *buf, void *pos, s32 a, s32 b, s32 c, s32 d);

void func_8004CE44(s32 id, void *pos, s16 a, s32 d)
{
    func_8004CC30(id, 0, pos, a, 8000, 10000, d);
}
