#include "ac1/ac1.h"
typedef struct {
    s8 count;
    s8 rows;
    u8 unk02[0x22];
    s16 top;            /* +0x24 */
} ShopListHdr;
extern ShopListHdr *D_801F5094[];
extern u8 D_800B8B58[];
extern void func_8008BF90(UiObj *o);

void func_8008BEF8(void)
{
    UiObj *o;

    o = ui_obj_alloc(2);
    o->ctor = func_8008BEF8;
    o->update = func_8008BF90;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 11;
    o->unk14 = 1;
    o->res = D_800B8B58;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->pos.vx = 288;
    o->pos.vy = D_801F5094[0]->top + 92;
    o->pos.vz = 0;
}
