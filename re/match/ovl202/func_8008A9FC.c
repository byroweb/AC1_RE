/* ovl202 0x8008A9FC  size 148  permuter tier (tools/loop/permute.py, decomp-permuter): from ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s8 D_801AC8A0;
extern s8 D_801AC8A1;
extern s8 D_801AC8A2;
extern s8 D_801AC8A3;

void func_8008A9FC(void)
{
  s8 temp_v1;
  int new_var;
  if (((u16) g_mission_end_timer) < 0x16U)
  {
    D_801AC8A3 = 2;
    if (((u16) g_mission_end_timer) < 3U)
    {
      new_var = 0xFF;
      D_801AC8A2 = new_var;
      D_801AC8A1 = new_var;
      D_801AC8A0 = new_var;
      return;
    }
    temp_v1 = ((0x16 - g_mission_end_timer) << 8) / 20;
    D_801AC8A2 = temp_v1;
    D_801AC8A1 = temp_v1;
    D_801AC8A0 = temp_v1;
  }
}
