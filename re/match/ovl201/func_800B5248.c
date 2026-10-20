/* ovl201 0x800B5248  size 148  permuter tier (tools/loop/permute.py): levers bytes arg0 + 4, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800B536C();                            /* extern */
extern s32 *D_800BE31C;
extern s32 *D_800BE320;
extern s32 *D_800BE324;
extern s32 *D_800BE34C;
extern s32 *D_800BE354;

void func_800B5248(s32 *arg0, u32 arg1) {
    func_800B536C();
    *D_800BE354 |= 0x88;
    *D_800BE31C = ((s32 *)((u8 *)arg0 + 4));
    *D_800BE320 = ((arg1 >> 5) << 0x10) | 0x20;
    *D_800BE34C = *arg0;
    *D_800BE324 = 0x01000201;
}
