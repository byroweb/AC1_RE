/* ovl201 0x800A4E38  size 116  permuter tier (tools/loop/permute.py, decomp-permuter): from ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 CdControl(M2C_UNK, s8 *, M2C_UNK);              /* extern */
s32 CdRead2(M2C_UNK);                               /* extern */
M2C_UNK VSync(M2C_UNK);                             /* extern */

void func_800A4E38(s8 *arg0)
{
  volatile char sp10;
  sp10 = 0x80;
  do
  {
    loop_1:
    if (CdControl(2, arg0, 0) == 0)
    {
      goto loop_1;
    }

    loop_3:
    if (CdControl(0xE, &sp10, 0) == 0)
    {
      goto loop_3;
    }

    VSync(3);
  }
  while (CdRead2((float) 0x1C0) == 0);
}
