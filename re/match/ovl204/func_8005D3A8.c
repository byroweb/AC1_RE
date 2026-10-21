/* ovl204 0x8005D3A8  size 212  zero-agent sweep (tools/loop/sweep.py): neighbour+imm draft of ovl203 0x8005DC1C (ovl203/func_8005DC1C.c, similarity 0.981), 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK MulMatrix2(M2C_UNK *, M2C_UNK *);           /* extern */
M2C_UNK SetBackColor(u8, u8, u8);                   /* extern */
M2C_UNK SetColorMatrix(s32);                        /* extern */
M2C_UNK SetLightMatrix(void *);                     /* extern */
M2C_UNK SetRotMatrix(M2C_UNK *);                    /* extern */
M2C_UNK SetTransMatrix(M2C_UNK *);                  /* extern */
M2C_UNK func_80013BE8(M2C_UNK, M2C_UNK *);          /* extern */
M2C_UNK func_80059720(M2C_UNK, M2C_UNK);            /* extern */
extern void *D_801ADCD8;
extern s32 D_801AE2EC;
extern M2C_UNK D_801B4F9C;

void func_8005D3A8(unsigned short arg0, M2C_UNK arg1, void *arg2)
{
  union 
  {
    s8 b[0x20];
    s32 w;
  } sp_frame;
  void *temp_v0;
  void *temp_v0_2;
  func_80013BE8(arg1, &(*((M2C_UNK *) (sp_frame.b + 0x0))));
  MulMatrix2(&D_801B4F9C, &(*((M2C_UNK *) (sp_frame.b + 0x0))));
  SetRotMatrix(&(*((M2C_UNK *) (sp_frame.b + 0x0))));
  *((s32 *) (sp_frame.b + 0x14)) = 0;
  *((s32 *) (sp_frame.b + 0x18)) = 0x1F4;
  *((s32 *) (sp_frame.b + 0x1C)) = 0;
  SetTransMatrix(&(*((M2C_UNK *) (sp_frame.b + 0x0))));
  SetColorMatrix(arg2 + 0x50);
  SetBackColor(*((u8 *) (((s8 *) arg2) + 0x62)), *((u8 *) (((s8 *) arg2) + 0x63)), *((u8 *) (((s8 *) arg2) + 0x64)));
  SetLightMatrix(arg2);
  temp_v0 = *((void **) (((s8 *) ((M2C_UNK *) (((u8 *) (&D_801B4F9C)) + ((arg0 & 0xFFFF) * 4)))) + (-0x72C0)));
  temp_v0_2 = temp_v0 + (*((s32 *) (((s8 *) temp_v0) + 8)));
  D_801ADCD8 = temp_v0_2;
  D_801AE2EC = *((s32 *) (((s8 *) temp_v0_2) + 0xC));
  func_80059720(0, 0x1F00);
}
