#include "ac1/ac1.h"
/* ovl201 0x80051248 - allocate a buffer for file (a, b) plus four zero bytes, load it */
s32 func_80015D80(s32 a, s32 b);
u8 *func_80050FC4(s32 size);
void func_800165E4(s32 a, s32 b, u8 *buf);

u8 *func_80051248(u16 a, u16 b)
{
    s32 n;
    u8 *buf;
    u8 *p;

    n = func_80015D80(a, b);
    buf = func_80050FC4(n + 4);
    p = buf + n;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p = 0;
    func_800165E4(a, b, buf);
    return buf;
}
