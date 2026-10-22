#include "ac1/ac1.h"
/* ovl201 0x8004F6DC - build the 3D label of a ring panel (model slot, or 7
 * for slot 2 when the func_8004DCC0 object's flag at +0xB1 is set), tilted
 * up unless the panel faces the front. */

typedef struct {
    s16 model;
    s16 unk2;
} LabelRes;

extern void func_8004DCC0(void);

UiObj *func_8004F6DC(s16 kind, s16 slot, UiObj *panel)
{
    UiObj *o;
    s16 m = slot;
    u16 light;

    o = ui_obj_alloc(kind);
    o->ctor = func_8004F6DC;
    o->update = ui_obj_nop2;
    o->destroy = ui_obj_nop;
    o->render_type = 0;
    if (slot == 2 && obj_find_by_handle(func_8004DCC0)->w.raw[0x35] != 0) {
        m = 7;
    }
    ((LabelRes *)&o->res)->model = m;
    ((LabelRes *)&o->res)->unk2 = 0;
    o->view = 1;
    light = 6;
    if (panel->rot.vy == 0) {
        light = 4;
        o->rot.vx = -256;
        o->rot.vy = 0;
        o->rot.vz = 0;
    } else {
        o->rot.vx = -1136;
        o->rot.vy = 0;
        o->rot.vz = 0;
    }
    o->light = light;
    o->pos.vy = -68;
    o->pos.vx = 0;
    o->pos.vz = -480;
    o->parent = panel;
    return o;
}
