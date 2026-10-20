/* ovl204 0x80077CD8  size 416  permuter tier (tools/loop/permute.py, decomp-permuter): from ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800771D0();                            /* extern */
M2C_UNK func_800775B8(void *, M2C_UNK);             /* extern */
M2C_UNK func_80077AA0();                            /* extern */
M2C_UNK func_80077C30();                            /* extern */
extern M2C_UNK *D_80039D18;
extern M2C_UNK D_80040FA0;
extern s32 D_80040FA8;
extern s32 D_80040FAC;
extern u32 D_800411F4;
extern s32 D_80041344;
extern s32 D_80041348;
extern u32 D_80041590;

void func_80077CD8(void)
{
  M2C_UNK (*temp_v0)(void *, M2C_UNK);
  M2C_UNK (*temp_v0_2)(void *, M2C_UNK);
  void *temp_s0;
  void *temp_s0_2;
  func_80077C30();
  func_800771D0();
  if ((D_80040FA8 & 0x800) && (!(D_80040FAC & 0x800)))
  {
    D_800411F4 = (D_800411F4 & 0xFFFEFFFF) | ((((((u32) D_800411F4) >> 0x10) ^ 1) & 1) << 0x10);
  }
  if ((D_80041344 & 0x800) && (!(D_80041348 & 0x800)))
  {
    D_80041590 = (D_80041590 & 0xFFFEFFFF) | ((((((u32) D_80041590) >> 0x10) ^ 1) & 1) << 0x10);
  }
  D_80039D18 = &D_80040FA0;
  func_80077AA0();
  temp_s0 = *((void **) (((s8 *) D_80039D18) + 0x394));
  temp_v0 = *((M2C_UNK (**)(void *, M2C_UNK)) (((s8 *) temp_s0) + 0x54));
  if (temp_v0 != ((void *) 0))
  {
    temp_v0(temp_s0, 0);
  }
  if ((*((s32 *) (((s8 *) temp_s0) + 0))) != 0)
  {
    func_800775B8(temp_s0, 0);
  }
  D_80039D18 = (temp_s0 = &D_80040FA0) + 0x39C;
  func_80077AA0();
  temp_s0_2 = *((void **) (((s8 *) D_80039D18) + 0x394));
  temp_v0_2 = *((M2C_UNK (**)(void *, M2C_UNK)) (((s8 *) temp_s0_2) + 0x54));
  if (temp_v0_2 != ((void *) 0))
  {
    temp_v0_2(temp_s0_2, 0);
  }
  if ((*((s32 *) (((s8 *) temp_s0_2) + 0))) != 0)
  {
    func_800775B8(temp_s0_2, 1);
  }
}
