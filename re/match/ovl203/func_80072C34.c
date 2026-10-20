/* ovl203 0x80072C34  size 100  permuter tier (tools/loop/permute.py, decomp-permuter): from ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80015538(M2C_UNK *, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_80072B20(M2C_UNK);                     /* extern */
M2C_UNK func_80072B30(M2C_UNK);                     /* extern */
extern M2C_UNK D_8019B550;
extern void *D_8019D830;
extern s32 D_8019D83C;
extern s32 D_8019D840;

void func_80072C34(void)
{
  func_80015538(&D_8019B550, 0, 0xB8);
  D_8019D830 = &D_8019B550;
  D_8019D830 = D_8019D830 + 0x2E0;
  D_8019D83C = 0;
  func_80072B20(0);
  D_8019D840 = 0;
  func_80072B30(0);
}
