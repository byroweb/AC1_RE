/* ovl201 0x800953AC  size 160  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8009544C(UiObj *);                        /* extern */
extern M2C_UNK D_800B8B9C;
extern s8 D_8019C092;
extern s8 D_8019C093;

void func_800953AC(void)
{
  UiObj *temp_v0;
  temp_v0 = ui_obj_alloc(6);
  temp_v0->ctor = func_800953AC;
  temp_v0->update = func_8009544C;
  temp_v0->destroy = ui_obj_nop;
  temp_v0->render_type = 3;
 do { temp_v0->light = 0x15; temp_v0->unk14 = 1; temp_v0->res = &D_800B8B9C; temp_v0->has_xform = 0; temp_v0->rot.vx = 0; temp_v0->rot.vy = 0; (*temp_v0).rot.vz = 0; } while (0);
  temp_v0->pos.vx = 0;
  temp_v0->pos.vz = 0;
  temp_v0->pos.vy = (4 * (D_8019C092 - D_8019C093)) * 6;
}
