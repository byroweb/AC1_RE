#include "ac1_types.h"
extern s32 func_80051248(s16 a, s16 b);
extern void func_80050944(s32 h);
extern void func_8002BFE8(s32 x);
extern void func_80051078(s32 h);
void func_80050804(s16 a, s16 b)
{
    s32 h;
    h = func_80051248(a, b);
    func_80050944(h);
    func_8002BFE8(0);
    func_80051078(h);
}
