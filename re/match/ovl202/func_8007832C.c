/* ovl202 0x8007832C  size 204  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800745D0(s32, u16 *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
void *func_80086854(M2C_UNK, u16 *, M2C_UNK *);     /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_8008E528;

void func_8007832C(s32 arg0, M2C_UNK arg1, M2C_UNK arg2)
{
  union 
  {
    s8 b[0x8];
    s32 w;
  } sp_frame;
  void *temp_s0;
  int new_var;
  if ((rand() < 0x2000) && (func_800745D0(arg0, &(*((u16 *) (sp_frame.b + 0x0))), arg1, arg2, 1, 0) != 0))
  {
    new_var = 0x10;
    *((u16 *) (sp_frame.b + 0x0)) = ((*((u16 *) (sp_frame.b + 0x0))) - new_var) + (rand() & 0x1F);
    *((u16 *) (sp_frame.b + 0x4)) = ((*((u16 *) (sp_frame.b + 0x4))) - new_var) + (rand() & 0x1F);
    temp_s0 = func_80086854(0, &(*((u16 *) (sp_frame.b + 0x0))), &D_8008E528);
    *((s16 *) (((s8 *) temp_s0) + 0xB8)) = (s16) ((rand() & 1) + 0x20);
  }
}
