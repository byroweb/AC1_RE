/* ovl201 0x8008AFA8  size 160  local LLM (tools/loop/golem.py tweak, qwen2.5-coder-7b-instruct), 2026-10-21 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8008B094(UiObj *);                        /* extern */
extern M2C_UNK D_800B8A80;
extern s8 D_801F37C9;
extern s8 D_801F37CA;

void func_8008AFA8(void)
{
  UiObj *temp_v0;
      temp_v0 = ui_obj_alloc(4);
  temp_v0->ctor = func_8008AFA8;
  temp_v0->update = func_8008B094;
  temp_v0->destroy = ui_obj_nop;
  temp_v0->render_type = 3;
 do { temp_v0->light = 0x15; temp_v0->unk14 = 1; temp_v0->res = &D_800B8A80; temp_v0->has_xform = 0; temp_v0->rot.vx = 0; temp_v0->rot.vy = 0; (*temp_v0).rot.vz = 0; } while (0);
  temp_v0->pos.vx = 0;
  temp_v0->pos.vz = 0;
  temp_v0->pos.vy = (4 * (D_801F37C9 - D_801F37CA)) * 6;
}
