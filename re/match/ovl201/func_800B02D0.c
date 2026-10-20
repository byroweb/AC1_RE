/* ovl201 0x800B02D0  size 212  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsSeqSetVol(s16, s32, s32);                 /* extern */
M2C_UNK SsSetMVol(s32, s32);                        /* extern */
M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern u8 D_80048661;
extern u8 D_80048662;
extern u8 D_801BC470;
extern M2C_UNK func_800B03A4;

void func_800B02D0(void *arg0)
{
  s32 temp_a0;
  s32 temp_a1;
  u8 *new_var;
  func_800506EC(8);
  obj_find_by_id(7)->kill = 1;
  if ((*((u8 *) (((s8 *) arg0) + 0x84))) == 0x4D)
  {
    func_8005C2D0(0xA8, 0x2C, 0x50, 0x10, 7);
    temp_a0 = D_80048661 * 2;
    SsSetMVol(temp_a0, temp_a0);
  }
  else
  {
    func_8005C2D0(0xA8, 0x44, 0x50, 0x10, 7);
    temp_a1 = D_80048662 * 2;
    new_var = &D_801BC470;
    SsSeqSetVol(*((s16 *) (((s8 *) ((&D_801BC470) + ((*new_var) * 2))) + (-0x1C))), temp_a1, temp_a1);
  }
  *((M2C_UNK **) (((s8 *) arg0) + 0x74)) = &func_800B03A4;
}
