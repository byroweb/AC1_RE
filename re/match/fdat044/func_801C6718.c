/* fdat044 0x801C6718  size 136  permuter tier (tools/loop/permute.py): decomp-permuter, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_8006E6B4(s16, void *);                     /* extern */

void func_801C6718(void *arg0)
{
  u16 *new_var;
  *((u16 *) (((s8 *) arg0) + 4)) = (u16) ((*((u16 *) (((s8 *) arg0) + 4))) + (*((u16 *) (((s8 *) arg0) + 0x26))));
  *((u16 *) (((s8 *) arg0) + 0x26)) = (u16) ((*((u16 *) (((s8 *) arg0) + 0x26))) + 4);
  new_var = (u16 *) (((s8 *) arg0) + 0xBA);
  *new_var = (u16) ((*new_var) + (*((u16 *) (((s8 *) arg0) + 0x18))));
  *((u16 *) (((s8 *) arg0) + 0x18)) = (u16) ((*((u16 *) (((s8 *) arg0) + 0x18))) + (*((u16 *) (((s8 *) arg0) + 0x1A))));
  *((s16 *) (((s8 *) arg0) + 0xE)) = func_8006E6B4(*((s16 *) (((s8 *) arg0) + 0xE)), arg0);
  if (((s16) (*new_var)) <= (*((s16 *) (((s8 *) arg0) + 0x1C))))
  {
    *((s16 *) (((s8 *) arg0) + 0x12)) = 0;
  }
}
