/* ovl201 0x800B52DC  size 144  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800B5404();                            /* extern */
extern s32 *D_800BE328;
extern s32 *D_800BE32C;
extern s32 *D_800BE330;
extern s32 *D_800BE354;

void func_800B52DC(s32 arg0, u32 arg1) {
    func_800B5404();
    *D_800BE354 |= 0x88;
    *D_800BE330 = 0;
    *D_800BE328 = arg0;
    *D_800BE32C = ((arg1 >> 5) << 0x10) | 0x20;
    *D_800BE330 = 0x01000200;
}
