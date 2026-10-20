/* ovl202 0x80053020  size 368  permuter tier (tools/loop/permute.py): decomp-permuter after levers etype D_801ED854 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK LoadAverageCol(M2C_UNK *, void *, s32, s32, void *); /* extern */
s32 rcos(M2C_UNK);                                  /* extern */
s32 rsin(M2C_UNK);                                  /* extern */
extern u8 D_80039D10;
extern M2C_UNK D_8008DBC8;
extern u8 D_801ED854;

void func_80053020(void *arg0, void *arg1, M2C_UNK arg2)
{
  s16 temp_s0_2;
  s16 temp_s1_2;
  s16 temp_v0_2;
  s16 temp_v1;
  s32 temp_s0;
  s32 temp_s1;
  s32 temp_s2;
  s32 temp_v0;
  s32 temp_v1_2;
  s32 temp_v1_3;
  if ((D_80039D10 != 0) || ((*((u8 *) (((s8 *) arg0) + 0xC))) & 4))
  {
    temp_s0 = ((s32) ((*((s16 *) (((s8 *) arg0) + 0))) + (*((s16 *) (((s8 *) arg0) + 4))))) >> 1;
    temp_s1 = ((s32) ((*((s16 *) (((s8 *) arg0) + 2))) + (*((s16 *) (((s8 *) arg0) + 6))))) >> 1;
    temp_s2 = rsin(arg2);
    temp_v0 = rcos(arg2);
    temp_v1 = temp_s0 + (((s32) (((*((s16 *) (((s8 *) arg0) + 0))) - temp_s0) * temp_s2)) >> 0xC);
    *((s16 *) (((s8 *) arg1) + 0x10)) = temp_v1;
    *((s16 *) (((s8 *) arg1) + 8)) = temp_v1;
    temp_v0 = 0x1000 - temp_v0;
    temp_v1_2 = temp_v0;
    temp_s0_2 = temp_s0 + (((s32) (((*((s16 *) (((s8 *) arg0) + 4))) - temp_s0) * temp_s2)) >> 0xC);
    *((s16 *) (((s8 *) arg1) + 0x14)) = temp_s0_2;
    *((s16 *) (((s8 *) arg1) + 0xC)) = temp_s0_2;
    temp_v1_3 = ((s32) (temp_v1_2 * temp_v1_2)) >> 0xC;
    temp_v0_2 = temp_s1 + (((s32) (((*((s16 *) (((s8 *) arg0) + 2))) - temp_s1) * temp_v1_3)) >> 0xC);
    *((s16 *) (((s8 *) arg1) + 0xE)) = temp_v0_2;
    *((s16 *) (((s8 *) arg1) + 0xA)) = temp_v0_2;
    temp_s1_2 = temp_s1 + (((s32) (((*((s16 *) (((s8 *) arg0) + 6))) - temp_s1) * temp_v1_3)) >> 0xC);
    *((s16 *) (((s8 *) arg1) + 0x16)) = temp_s1_2;
    *((s16 *) (((s8 *) arg1) + 0x12)) = temp_s1_2;
    LoadAverageCol(&D_8008DBC8, ((*((u8 *) (((s8 *) arg0) + 0xE))) * 4) + (&D_801ED854), 0x1000 - temp_s2, temp_s2, arg1 + 4);
  }
}
