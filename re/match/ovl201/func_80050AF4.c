#include "ac1/ac1.h"
/* ovl201 0x80050AF4 - func_80050A54 with an async read: free slot i, allocate, start
 * the read of file id of archive 5 with func_80050B9C as the completion callback */
extern u8 *D_801F36D0[];
extern s32 D_801BC478[];
void func_80051078(u8 *p);
s32 func_80015C9C(s32 a, u16 b, u8 *loc);
u8 *func_80050FC4(s32 size);
void func_80015B78(s32 a, u8 *loc, s32 size, u8 *buf, void (*cb)());
void func_80050B9C();

void func_80050AF4(s16 i, s16 id)
{
    u8 loc[8];
    s32 n;
    u8 *p;

    if (D_801F36D0[i] != 0) {
        func_80051078(D_801F36D0[i]);
    }
    n = func_80015C9C(5, id, loc);
    p = func_80050FC4(n);
    D_801F36D0[i] = p;
    D_801BC478[i] = n;
    func_80015B78(16, loc, n, p, func_80050B9C);
}
