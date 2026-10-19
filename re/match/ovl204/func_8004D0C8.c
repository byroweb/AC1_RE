#include "ac1_types.h"
/* ovl204 func_8004D0C8 - forwards to func_8004D278, passing the second argument
 * as the third or the fourth parameter depending on whether the global pointer
 * D_80039D18 points at D_80040FA0. */

extern u8 D_80040FA0[];
extern u8 *D_80039D18;

extern void func_8004D278(s32 a, s32 b, s32 c, s32 d, s32 e);

void func_8004D0C8(s32 a, s32 b)
{
    if (D_80039D18 == D_80040FA0) {
        func_8004D278(a, 0, b, 0, 0);
    } else {
        func_8004D278(a, 0, 0, b, 0);
    }
}
