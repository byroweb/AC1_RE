/* ovl201 0x800B1980  size 196  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800B1A44(UiObj *);                        /* extern */
extern M2C_UNK D_800BDDF4;

void func_800B1980(u8 *arg0, u8 *arg1)
{
  UiObj *temp_v0;
  temp_v0 = ui_obj_alloc(8);
  temp_v0->ctor = func_800B1980;
  temp_v0->update = func_800B1A44;
  temp_v0->destroy = ui_obj_nop;
  temp_v0->render_type = 3;
  temp_v0->light = 0x15;
  temp_v0->unk14 = 1;
  temp_v0->res = &D_800BDDF4;
  temp_v0->has_xform = 0;
  temp_v0->rot.vx = 0;
  temp_v0->rot.vy = 0;
  temp_v0->rot.vz = 0;
  temp_v0->pos.vx = (*arg0) * 0x90;
  temp_v0->pos.vy = ((*arg1) * 8) + 0xC;
  temp_v0->pos.vz = 0;
  temp_v0->w.list.src = arg0;
  *((u8 **) (((s8 *) temp_v0) + 0x80)) = arg1;
}
