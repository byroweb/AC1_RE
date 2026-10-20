/* ovl201 0x8004F098  size 304  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8004F1C8(UiObj *);                        /* extern */
extern s16 D_800B8E64;
extern s16 D_800B8E66;
extern s16 D_800B8E68;
extern s16 D_800B8E6A;
extern s16 D_800B8E6C;
extern s16 D_800B8E6E;
extern s16 D_800B8E70;
extern s16 D_800B8E72;
extern s16 D_800B8E74;

void func_8004F098(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u16 arg4, u16 arg5, s16 arg6) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc((s32) arg6);
    temp_v0->ctor = func_8004F098;
    temp_v0->update = func_8004F1C8;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->light = 5;
    temp_v0->scaled = 1;
    temp_v0->render_type = 0;
    M2C_FIELD(temp_v0, s16 *, 0xC) = 0;
    M2C_FIELD(temp_v0, s16 *, 0xE) = 0;
    temp_v0->scale.vx = 0x1000;
    temp_v0->scale.vy = 0x1000;
    temp_v0->scale.vz = 0x1000;
    temp_v0->rot.vx = arg3;
    temp_v0->rot.vy = (s16) arg4;
    temp_v0->rot.vz = (s16) arg5;
    temp_v0->pos.vx = (s32) arg0;
    temp_v0->pos.vy = (s32) arg1;
    temp_v0->pos.vz = (s32) arg2;
    D_800B8E64 = 0;
    D_800B8E66 = 0;
    D_800B8E68 = 0;
    D_800B8E6A = 0;
    D_800B8E6C = 0;
    D_800B8E6E = 0;
    D_800B8E70 = 0;
    D_800B8E72 = 0;
    D_800B8E74 = 0;
}
