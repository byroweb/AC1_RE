#include "ac1/ac1.h"

extern u8 func_8001588C(u8 v);

void func_8001594C(u8 *pos, u32 sec)
{
    pos[0] = func_8001588C(sec / 4500);
    pos[1] = func_8001588C(sec % 4500 / 75);
    pos[2] = func_8001588C(sec % 4500 % 75);
    pos[3] = 0;
}
