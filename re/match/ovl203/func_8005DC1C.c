/* ovl203 0x8005DC1C  size 212  permuter tier (tools/loop/permute.py): decomp-permuter after levers bytes &D_801A4C04 + E, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK MulMatrix2(M2C_UNK *, M2C_UNK *);           /* extern */
M2C_UNK SetBackColor(u8, u8, u8);                   /* extern */
M2C_UNK SetColorMatrix(s32);                        /* extern */
M2C_UNK SetLightMatrix(void *);                     /* extern */
M2C_UNK SetRotMatrix(M2C_UNK *);                    /* extern */
M2C_UNK SetTransMatrix(M2C_UNK *);                  /* extern */
M2C_UNK func_80013BE8(M2C_UNK, M2C_UNK *);          /* extern */
M2C_UNK func_80059F6C(M2C_UNK, M2C_UNK);            /* extern */
extern void *D_8019D968;
extern s32 D_8019DF7C;
extern M2C_UNK D_801A4C04;

void func_8005DC1C(unsigned short arg0, M2C_UNK arg1, void *arg2)
{
  union 
  {
    s8 b[0x20];
    s32 w;
  } sp_frame;
  void *temp_v0;
  void *temp_v0_2;
  func_80013BE8(arg1, &(*((M2C_UNK *) (sp_frame.b + 0x0))));
  MulMatrix2(&D_801A4C04, &(*((M2C_UNK *) (sp_frame.b + 0x0))));
  SetRotMatrix(&(*((M2C_UNK *) (sp_frame.b + 0x0))));
  *((s32 *) (sp_frame.b + 0x14)) = 0;
  *((s32 *) (sp_frame.b + 0x18)) = 0x1F4;
  *((s32 *) (sp_frame.b + 0x1C)) = 0;
  SetTransMatrix(&(*((M2C_UNK *) (sp_frame.b + 0x0))));
  SetColorMatrix(arg2 + 0x50);
  SetBackColor(*((u8 *) (((s8 *) arg2) + 0x62)), *((u8 *) (((s8 *) arg2) + 0x63)), *((u8 *) (((s8 *) arg2) + 0x64)));
  SetLightMatrix(arg2);
  temp_v0 = *((void **) (((s8 *) ((M2C_UNK *) (((u8 *) (&D_801A4C04)) + ((arg0 & 0xFFFF) * 4)))) + (-0x7298)));
  temp_v0_2 = temp_v0 + (*((s32 *) (((s8 *) temp_v0) + 8)));
  D_8019D968 = temp_v0_2;
  D_8019DF7C = *((s32 *) (((s8 *) temp_v0_2) + 0xC));
  func_80059F6C(0, 0x1F00);
}
