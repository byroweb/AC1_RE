/* fdat074 0x801C6FA0  size 496  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-22 */
#include "ac1/ac1.h"

typedef struct { s16 a, b, c; } Blk6;
typedef struct { s16 x, y, z, w; } V8;
typedef struct {
    u8 pad0[0x12];
    s16 f12;
} Eff;
extern s32 func_80014D78();
extern s32 func_80052338();
extern void func_800529F0();
extern Eff *func_800862E8();
extern void func_8008A80C();
extern s32 func_801C6EF4();
extern u8 D_80039C78[];
extern V8 D_80198C14;
extern u16 D_8019F528;
extern u8 D_801A26B8[];
extern s32 D_801C7314;
extern s32 D_801C7318;
extern s32 D_801C731C;
extern Blk6 D_801C4D48;

void func_801C6FA0(void)
{
  Blk6 sp28;
  V8 sp30;
  u8 *new_var;
  s32 n;
  s32 r;
  Eff *p;
  func_8008A80C();
  if (D_8019F528 == 0)
  {
    if (D_801C7314 == 0x528)
    {
      func_800529F0(0, 0, 0x54);
    }
    else
      if (((D_801C7314 >= 0x529) && (D_801C731C < 3)) && (func_80052338() != 0))
    {
      if (D_801C7318 == 0)
      {
        sp28 = D_801C4D48;
        n = 6;
        do
        {
          new_var = (u8 *) (&D_80198C14);
          r = func_801C6EF4();
          sp30 = D_80198C14;
          sp30.x = ((s16) (sp30.x - 0x100)) + func_80014D78(0x200);
          sp30.y = ((s16) (sp30.y - 0x100)) + func_80014D78(0x200);
          if (r != 0)
          {
            p = func_800862E8(D_801A26B8, &sp28, new_var - 8, &sp30, D_80039C78, 0x30, 10, 0, r);
            p->f12 = 200;
          }
          n--;
        }
        while (n != 0);
        D_801C7318 = 0x42;
        D_801C731C++;
      }
      else
      {
        D_801C7318--;
      }
    }
    if (D_801C7314 < 0xFFFF)
    {
      D_801C7314++;
    }
  }
}
