/* fdat008 0x801C6884  size 556  permuter tier (tools/loop/permute.py): decomp-permuter after levers etype D_80198B08 M2C_UNK->u8, from the ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_800132CC(s16, s16, s16);                   /* extern */
M2C_UNK func_80014338(s16 *, u16, s16 *);           /* extern */
M2C_UNK func_80014AE4(s16, s16, s16, s16 *);        /* extern */
s16 func_8006E6B4(s16, void *);                     /* extern */
extern u8 D_80198B08;

void func_801C6884(void *arg0)
{
  union 
  {
    s8 b[0x10];
    s32 w;
  } sp_frame;
  s16 temp_a0;
  s16 temp_a0_2;
  s16 temp_a0_3;
  s16 temp_a0_4;
  s16 temp_a1;
  s16 temp_a1_2;
  s16 temp_a1_3;
  s16 temp_v0;
  u16 temp_v1;
  u8 temp_a0_5;
  void *temp_s1;
  void *temp_s2;
  temp_s2 = *((void **) (((s8 *) arg0) + 0x14));
  temp_s1 = *((void **) (((s8 *) temp_s2) + 0xC));
  temp_a0 = (*((u16 *) (((s8 *) temp_s1) + 0))) - (*((u16 *) (((s8 *) arg0) + 0)));
  *((s16 *) (sp_frame.b + 0x0)) = temp_a0;
  temp_a1 = (*((u16 *) (((s8 *) temp_s1) + 2))) - (*((u16 *) (((s8 *) arg0) + 2)));
  *((s16 *) (sp_frame.b + 0x2)) = temp_a1;
  temp_v0 = (*((u16 *) (((s8 *) temp_s1) + 4))) - (*((u16 *) (((s8 *) arg0) + 4)));
  *((s16 *) (sp_frame.b + 0x4)) = temp_v0;
  func_80014AE4(temp_a0, temp_a1, temp_v0, &(*((s16 *) (sp_frame.b + 0x8))));
  *((s16 *) (((s8 *) arg0) + 8)) = func_800132CC(*((s16 *) (((s8 *) arg0) + 8)), *((s16 *) (sp_frame.b + 0x8)), *((s16 *) (((s8 *) arg0) + 0x2C)));
  *((s16 *) (((s8 *) arg0) + 0xA)) = func_800132CC(*((s16 *) (((s8 *) arg0) + 0xA)), (s16) ((*((u16 *) (sp_frame.b + 0xA))) - 0x400), *((s16 *) (((s8 *) arg0) + 0x2C)));
  *((s16 *) (((s8 *) arg0) + 0xC)) = func_800132CC(*((s16 *) (((s8 *) arg0) + 0xC)), *((s16 *) (sp_frame.b + 0xC)), *((s16 *) (((s8 *) arg0) + 0x2C)));
  func_80014338(&(*((s16 *) (sp_frame.b + 0x0))), *((u16 *) (((s8 *) arg0) + 0x2A)), &(*((s16 *) (sp_frame.b + 0x0))));
  *((u16 *) (((s8 *) arg0) + 0)) = (u16) ((*((u16 *) (((s8 *) arg0) + 0))) + ((u16) (*((s16 *) (sp_frame.b + 0x0)))));
  *((u16 *) (((s8 *) arg0) + 2)) = (u16) ((*((u16 *) (((s8 *) arg0) + 2))) + ((u16) (*((s16 *) (sp_frame.b + 0x2)))));
  temp_v1 = (*((u16 *) (((s8 *) arg0) + 4))) + ((u16) (*((s16 *) (sp_frame.b + 0x4))));
  *((u16 *) (((s8 *) arg0) + 4)) = temp_v1;
  temp_a1_2 = (s16) (*((u16 *) (((s8 *) temp_s1) + 0)));
  temp_a0_2 = (s16) (*((u16 *) (((s8 *) arg0) + 0)));
  if ((temp_a0_2 >= (temp_a1_2 - 0x20)) && ((temp_a1_2 + 0x20) >= temp_a0_2))
  {
    temp_a1_3 = (s16) (*((u16 *) (((s8 *) temp_s1) + 2)));
    temp_a0_3 = (s16) (*((u16 *) (((s8 *) arg0) + 2)));
    if ((temp_a0_3 >= (temp_a1_3 - 0x28)) && ((temp_a1_3 + 0x20) >= temp_a0_3))
    {
      temp_a0_4 = (s16) (*((u16 *) (((s8 *) temp_s1) + 4)));
      if ((((s16) temp_v1) >= (temp_a0_4 - 0x20)) && ((temp_a0_4 + 0x20) >= ((s16) temp_v1)))
      {
        temp_a0_5 = *((u8 *) (((s8 *) temp_s1) + 0x28));
        if (temp_a0_5 != 0)
        {
          *((void **) (((s8 *) temp_s2) + 0xC)) = (void *) ((temp_a0_5 * 0x34) + (&D_80198B08));
        }
        else
        {
          temp_s1 = ((*((u16 *) (((s8 *) arg0) + 0x28))) * 0x34) + (&D_80198B08);
          *((void **) (((s8 *) temp_s2) + 0xC)) = temp_s1;
          *((u16 *) (((s8 *) arg0) + 0)) = (u16) (*((u16 *) (((s8 *) temp_s1) + 0)));
          *((u16 *) (((s8 *) arg0) + 2)) = (u16) (*((u16 *) (((s8 *) temp_s1) + 2)));
          *((u16 *) (((s8 *) arg0) + 4)) = (u16) (*((u16 *) (((s8 *) temp_s1) + 4)));
        }
      }
    }
  }
  *((s16 *) (((s8 *) arg0) + 0xE)) = func_8006E6B4(*((s16 *) (((s8 *) arg0) + 0xE)), arg0);
}
