/* fdat002 (15 twins): call func_800759A4(a, b, &off) with a random horizontal offset
 * of -50..49 on x and z. */
#include "ac1/ac1.h"
extern s32 func_80014D78(s32 n);
extern void func_800759A4(void *a, void *b, SVECTOR *off);
void func_801C5978(void *a, void *b)
{
    SVECTOR off;

    off.vx = func_80014D78(100) - 50;
    off.vz = func_80014D78(100) - 50;
    off.vy = 0;
    func_800759A4(a, b, &off);
}
