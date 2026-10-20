/* ovl201 0x80095784  size 160  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80095824(UiObj *);                        /* extern */
extern M2C_UNK D_800B8BC0;
extern s16 D_8019C0EA;
extern s16 D_8019C104;

void func_80095784(void)
{
  int new_var;
  UiObj *temp_v0;
  temp_v0 = ui_obj_alloc(0xA);
  temp_v0->ctor = func_80095784;
  temp_v0->update = func_80095824;
  temp_v0->destroy = ui_obj_nop;
  temp_v0->render_type = 3;
  (*temp_v0).light = 0x15;
  temp_v0->unk14 = 1;
  temp_v0->res = &D_800B8BC0;
  temp_v0->has_xform = 0;
 do { temp_v0->rot.vx = 0; new_var = 0; temp_v0->rot.vy = new_var; do { temp_v0->rot.vz = 0; } while (0); temp_v0->pos.vx = 0; temp_v0->pos.vz = 0; temp_v0->pos.vy = (D_8019C0EA - D_8019C104) * 0x18; } while (0);
}
