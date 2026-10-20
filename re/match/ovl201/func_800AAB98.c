/* ovl201 0x800AAB98  size 136  permuter tier (tools/loop/permute.py, decomp-permuter): from ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s8 D_80039C5D;
extern u8 D_80039CAE;
extern s8 D_80039D2A;
extern u8 D_800411F8;
extern u8 D_80041594;
extern s16 D_801A2560;
extern s8 D_801A2570;
extern M2C_UNK func_8005C2D0;

void func_800AAB98(void *arg0)
{
  int new_var;
  if (obj_find_by_handle(&func_8005C2D0) == (&g_ui_none))
  {
    new_var = 0xFF;
    D_80039C5D = new_var;
    D_80039D2A = 0;
    D_800411F8 = D_80039CAE;
    D_80041594 = D_80039CAE;
    *((s8 *) (((s8 *) arg0) + 1)) = 1;
    D_801A2570 = 0;
    D_801A2560 = 1;
  }
}
