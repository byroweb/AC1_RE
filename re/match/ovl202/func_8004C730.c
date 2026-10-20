/* ovl202 0x8004C730  size 120  permuter tier (tools/loop/permute.py): decomp-permuter after levers etype D_80041BC2 s8->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_80041BA0;
extern u8 D_80041BB8;
extern u8 D_80041BBA;
extern u8 D_80041BBB;
extern u8 D_80041BBD;
extern u8 D_80041BC2;
extern s8 D_80041BC8;
extern s8 D_80041BC9;
extern s8 D_80041BCA;
extern s8 D_80041BCB;
extern s8 D_80041BCC;
extern s8 D_80041BCD;
extern s8 D_80041BCE;

void func_8004C730(void)
{
  unsigned int new_var;
  new_var = 0xFF;
  D_80041BCE = new_var;
  D_80041BCD = new_var;
  D_80041BCC = new_var;
  D_80041BCB = new_var;
  D_80041BCA = new_var;
  D_80041BC9 = new_var;
  D_80041BC8 = new_var;
  D_80041BA0 = 0;
  D_80041BC2 = 0xF0;
  D_80041BBB = D_80041BB8;
  D_80041BBD = D_80041BBA;
}
