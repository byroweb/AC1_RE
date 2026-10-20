/* ovl201 0x8008C1F8  size 336  permuter tier (tools/loop/permute.py): decomp-permuter, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8008FA94(UiObj *);                        /* extern */
void func_80093AE8(UiObj *);                        /* extern */
void func_80093E3C(UiObj *);                        /* extern */
void func_80095824(UiObj *);                        /* extern */
extern void *D_8019C164;
extern void *D_8019C168;
extern void *D_8019C16C;
extern void *D_8019C188;
extern void *D_8019C18C;
extern void *D_8019C190;
extern void *D_8019C194;
extern void *D_8019C198;
extern void *D_8019C19C;
extern M2C_UNK func_8008E70C;
extern M2C_UNK func_80090C9C;
extern M2C_UNK func_80093B58;
extern M2C_UNK func_80095784;

void func_8008C1F8(void)
{
  UiObj *temp_a1;
  UiObj *temp_s0;
  UiObj *temp_s1;
  UiObj *temp_s2;
  temp_s0 = obj_find_by_handle(&func_80095784);
  temp_s1 = obj_find_by_handle(&func_8008E70C);
  temp_s2 = obj_find_by_handle(&func_80090C9C);
 do { temp_a1 = obj_find_by_handle(&func_80093B58); temp_s2->update = func_80093AE8; temp_a1->update = func_80093E3C; } while (0);
  *((s16 *) (((s8 *) D_8019C164) + 0x16)) = 0x15;
  *((s16 *) (((s8 *) D_8019C168) + 0x16)) = 0x15;
  *((s16 *) (((s8 *) D_8019C16C) + 0x16)) = 0x15;
  *((s16 *) (((s8 *) D_8019C188) + 0x16)) = 0x14;
  *((s16 *) (((s8 *) D_8019C18C) + 0x16)) = 0x14;
  *((s16 *) (((s8 *) D_8019C190) + 0x16)) = 0x14;
  *((s16 *) (((s8 *) D_8019C194) + 0x16)) = 0x14;
  *((s16 *) (((s8 *) D_8019C198) + 0x16)) = 0x14;
  *((s16 *) (((s8 *) D_8019C19C) + 0x16)) = 0x14;
  temp_s1->update = func_8008FA94;
  temp_s0->update = func_80095824;
  temp_s0->light = 0x15;
  if (temp_s2 != (&g_ui_none))
  {
    temp_s2->update = func_80093AE8;
  }
  if (temp_a1 != (&g_ui_none))
  {
    temp_a1->update = func_80093E3C;
  }
}
