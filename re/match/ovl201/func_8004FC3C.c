#include "ac1/ac1.h"
/* ovl201 0x8004FC3C - build a type-3 object showing entry kind of the
 * D_800B6DF8 table (kind 1 becomes 6 when the func_8004DCC0 object's flag
 * at +0xB1 is set). */

typedef struct {
    u8 raw[0xC];
} Entry0C;

extern Entry0C D_800B6DF8[];
extern void func_8004FC34(UiObj *o);
extern void func_8004DCC0(void);

UiObj *func_8004FC3C(s16 kind)
{
    UiObj *o;
    s16 k = kind;

    o = ui_obj_alloc(2);
    o->ctor = func_8004FC3C;
    o->update = func_8004FC34;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 4;
    o->unk14 = 2;
    if (kind == 1 && obj_find_by_handle(func_8004DCC0)->w.raw[0x35] != 0) {
        k = 6;
    }
    o->res = &D_800B6DF8[k];
    o->pos.vx = 112;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->pos.vy = 120;
    o->pos.vz = 0;
    return o;
}
