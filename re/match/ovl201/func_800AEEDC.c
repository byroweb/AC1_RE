/* ovl201 0x800AEEDC  size 80  permuter tier (tools/loop/permute.py): levers index D_800BDEA8 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005CB94(s32, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */

extern u8 D_800BDEA8[];
void func_800AEEDC(void *arg0) {
    func_800506EC(6);
    func_8005CB94((*(s32 *)(D_800BDEA8 + ((M2C_FIELD(arg0, u8 *, 0x88) * 4)))), 8, 4, 6);
}
