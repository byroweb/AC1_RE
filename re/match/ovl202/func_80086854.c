/* ovl202 0x80086854  size 412  permuter tier (tools/loop/permute.py): decomp-permuter after levers swap temp_v0->ang[2] = 0x64; / temp_v0->ang[1] = ((u32) + bytes E + &D_80090128 + bytes E + &D_80090164, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004CE44(u16, AcPos *, s16, s16);      /* extern */
void func_8008523C(Projectile *);                   /* extern */
extern M2C_UNK D_80090128;
extern M2C_UNK D_80090164;

Projectile *func_80086854(void *arg0, AcPos *arg1, u32 *arg2)
{
  Projectile *temp_v0;
  u32 new_var;
  int new_var2;
  u32 temp_a0;
  u8 *temp_s0;
  int temp_v1;
  void *temp_v0_2;
  if ((((*arg2) & 7) == 7) || ((temp_v0 = proj_alloc(0), temp_v0 == (&g_proj_none))))
  {
    return &g_proj_none;
  }
  temp_a0 = *arg2;
  if ((temp_a0 & 0xF8000000) != 0xF8000000)
  {
    temp_v0_2 = (M2C_UNK *) (((u8 *) (&D_80090128)) + (((new_var = temp_a0) >> 0x1B) * 8));
    func_8004CE44(*((u16 *) (((s8 *) temp_v0_2) + 0)), arg1, *((s16 *) (((s8 *) temp_v0_2) + 4)), *((s16 *) (((s8 *) temp_v0_2) + 6)));
  }
  temp_s0 = (M2C_UNK *) (((u8 *) (&D_80090164)) + (((*arg2) & 7) * 8));
  proj_init(temp_v0, arg0, arg1, -1);
  temp_v0->kind = 2;
  temp_v0->ang[0] = (s16) (*temp_s0);
  temp_v0->dtint[0] = (*arg2) & 7;
  temp_v0->ang[1] = (((u32) (*arg2)) >> 0x11) & 0x3FF;
  temp_v0->ang[2] = 0x64;
  temp_v0->unk1b = (((u32) (*arg2)) >> 0xA) & 7;
  temp_v0->dtint[1] = (((u32) (*arg2)) >> 3) & 3;
  temp_v1 = (((u32) (*arg2)) >> 0xD) & 0xF;
  *((u8 *) (((s8 *) temp_v0) + 0x1C)) = temp_v1;
  temp_v0->unk12 = (((((u32) (*arg2)) >> 5) & (new_var2 = 0x1F)) + temp_v1) - 1;
  if ((*((u8 *) (((s8 *) temp_v0) + 0x1C))) != 0)
  {
    temp_v0->free = 1;
  }
  temp_v0->think = func_8008523C;
  temp_v0->dtint[2] = 0;
  return temp_v0;
}
