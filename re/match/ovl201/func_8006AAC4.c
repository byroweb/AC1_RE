/* ovl201 0x8006AAC4  emblem sample picker: scroll up one row (twin of 0x8006AB34) */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8006A2C4(void *);
M2C_UNK func_8006BA08();
extern M2C_UNK func_8006A3EC;

void func_8006AAC4(void *arg0)
{
  u16 temp_v0;
  func_8006BA08();
  temp_v0 = (*((u16 *) (((s8 *) arg0) + 0xC8))) - 0x10;
  *((u16 *) (((s8 *) arg0) + 0xC8)) = temp_v0;
  if (((s16) temp_v0) <= ((*((u8 *) (((s8 *) arg0) + 0xC5))) * 0x44))
  {
    *((u16 *) (((s8 *) arg0) + 0xC8)) = (u16) ((*((u8 *) (((s8 *) arg0) + 0xC5))) * 0x44);
    *((M2C_UNK **) (((s8 *) arg0) + 0x74)) = &func_8006A3EC;
  }
  func_8006A2C4(arg0);
}
