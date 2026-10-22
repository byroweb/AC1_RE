#include "ac1/ac1.h"
/* ovl201 0x80050A54 - (re)load file id of archive 5 into slot i: free the old buffer,
 * load the new one, keep its size, func_800B597C on the data */
extern u8 *D_801F36D0[];
extern s32 D_801BC478[];
void func_80051078(u8 *p);
u8 *func_80051248(s32 a, s16 b);
s32 func_80015D80(s32 a, u16 b);
void func_800B597C(u8 *p);

void func_80050A54(s16 i, s16 id)
{
    u8 *p;

    if (D_801F36D0[i] != 0) {
        func_80051078(D_801F36D0[i]);
    }
    p = func_80051248(5, id);
    D_801F36D0[i] = p;
    D_801BC478[i] = func_80015D80(5, id);
    func_800B597C(p + 4);
}
