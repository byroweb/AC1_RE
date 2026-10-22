#include "ac1/ac1.h"

void func_800998B4(UiObj *o);
extern u8 D_800B8CDC[];
typedef struct { s8 unk0; s8 unk1; s8 unk2; } MailState;
extern MailState D_8017F794;

void func_800997C8(void)
{
    UiObj *o;

    o = ui_obj_alloc(4);
    o->ctor = func_800997C8;
    o->update = func_800998B4;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 21;
    o->unk14 = 1;
    o->res = D_800B8CDC;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->pos.vx = 0;
    o->pos.vy = (D_8017F794.unk1 - D_8017F794.unk2) * 24;
    o->pos.vz = 0;
}
