/* ovl201 0x80084A8C  size 88  permuter tier (tools/loop/permute.py): levers index D_800B8A30 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005CB94(s32, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
s32 func_80084364(s32);                             /* extern */

extern u8 D_800B8A30[];
void func_80084A8C(s32 arg0) {
    func_800506EC(0xA);
    func_8005CB94((*(s32 *)(D_800B8A30 + (((func_80084364(arg0) & 0xFF) * 4)))), 4, 4, 0xA);
}
