#include "ac1/ac1.h"
typedef struct {
    s8 count;
    s8 rows;
    u8 unk02[0x22];
    s16 top;            /* +0x24 */
} ShopListHdr;
extern ShopListHdr *D_801F5094[];
extern u8 D_800B8B4C[];
extern s16 D_800B8F84[9];
extern void func_8008BE98(UiObj *o);

UiObj *func_8008BDA8(void)
{
    UiObj *o;

    o = ui_obj_alloc(2);
    o->ctor = func_8008BDA8;
    o->update = func_8008BE98;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 14;
    o->unk14 = 1;
    o->res = D_800B8B4C;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->pos.vx = 232;
    o->pos.vy = D_801F5094[0]->top + 76;
    o->pos.vz = 0;
    D_800B8F84[0] = 32;
    D_800B8F84[1] = 32;
    D_800B8F84[2] = 32;
    D_800B8F84[3] = 128;
    D_800B8F84[4] = 128;
    D_800B8F84[5] = 128;
    D_800B8F84[6] = 256;
    D_800B8F84[7] = 256;
    D_800B8F84[8] = 256;
    return o;
}
