#include "ac1/ac1.h"

/* ovl203 copy of ovl202 func_8008A344 (mission init): the MissionCtl block and the
 * heap cursor sit at this overlay's own addresses. */
extern s16 func_80072A98(void);
extern MissionCtl D_801C40C8;
extern u8 *D_801DC0F0[];
extern s32 D_80048614;

void func_80088F84(void)
{
    u8 *heap;

    D_801C40C8.unk00 = func_80072A98();
    D_801C40C8.unk02 = 1200;
    heap = D_801DC0F0[0];
    D_801C40C8.vm_set = 0xFF;
    D_801DC0F0[0] = 0;
    D_80048614 = 0;
    D_801C40C8.heap = (u8 *)(((s32)heap + 3) & ~3);
}
