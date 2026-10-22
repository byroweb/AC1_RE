/* fdat042 0x801C6D64  size 216  permuter tier (tools/loop/permute.py): decomp-permuter after levers args func_80075030 +arg0 noproto, from the ctx-fix-frame draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_80075030();                       /* extern */
s32 func_800754D0();               /* extern */
M2C_UNK func_80078E1C();                      /* extern */
extern s32 D_801A5DC8;
extern M2C_UNK D_801CD638;
extern M2C_UNK D_801D0B68;

void func_801C6D64(void *arg0)
{
  s32 var_v0;
  u16 temp_v0_2;
  void *temp_v0;
  volatile long pad;
  D_801A5DC8 = 1;
  temp_v0 = func_80075030(arg0, 0x1800);
  if (temp_v0 != ((void *) 0))
  {
    *((u16 *) (((s8 *) arg0) + 0x2C)) = (u16) (*((u16 *) (((s8 *) temp_v0) + 0xE)));
    var_v0 = ((s32) ((*((s32 *) (((s8 *) ((s32) temp_v0)) + 4))) - ((s32) (&D_801D0B68)))) >> 6;
  }
  else
  {
    *((u16 *) (((s8 *) arg0) + 0x2C)) = 0x7FFFU;
    var_v0 = -1;
  }
  *((s16 *) (((s8 *) arg0) + 0x30)) = (s16) var_v0;
  D_801A5DC8 = 0;
  if ((*((u8 *) (((s8 *) arg0) + 0x40))) != 0xFF)
  {
    temp_v0_2 = (*((u16 *) (((s8 *) arg0) + 0xA))) + 0x32;
    *((u16 *) (((s8 *) arg0) + 0xA)) = temp_v0_2;
    if (((s16) temp_v0_2) >= ((s16) (*((u16 *) (((s8 *) arg0) + 0x2C)))))
    {
      *((u8 *) (((s8 *) arg0) + 0x40)) = 0U;
    }
    if (func_800754D0(arg0, &D_801CD638) != 0)
    {
      func_80078E1C(arg0);
    }
  }
}
