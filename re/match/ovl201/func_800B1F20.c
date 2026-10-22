#include "ac1/ac1.h"
/* ovl201 0x800B1F20 - copy the 15 words of D_800BE0C8 to D_80049E74 and
 * clear D_80049E70. */

extern s32 D_80049E70;
extern s32 D_80049E74[15];
extern s32 D_800BE0C8[15];

void func_800B1F20(void)
{
    s16 i;

    for (i = 0; i < 15; i++) {
        D_80049E74[i] = D_800BE0C8[i];
    }
    D_80049E70 = 0;
}
