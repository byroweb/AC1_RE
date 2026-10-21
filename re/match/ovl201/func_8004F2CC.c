#include "ac1/ac1.h"
/* ovl201 0x8004F2CC - build one of the six ring panels: its place on the
 * ring (angle and position) is slot - (front + 1), wrapped into -5..5;
 * func_8004F6DC builds its label. */

extern void func_8004F4A4(UiObj *o);
extern UiObj *func_8004F6DC(s16 kind, s16 slot, UiObj *o);

UiObj *func_8004F2CC(s16 kind, s16 slot, s8 front)
{
    UiObj *o;
    s8 f;

    o = ui_obj_alloc(kind);
    o->ctor = func_8004F2CC;
    o->update = func_8004F4A4;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->w.panel.state = 0;
    o->w.panel.unk7f = 0;
    f = front + 1;
    switch (slot - f) {
    case 0:
        o->rot.vx = 256;
        o->rot.vy = 0;
        o->rot.vz = 0;
        o->pos.vx = 0;
        o->pos.vy = 448;
        o->pos.vz = -3036;
        break;
    case -5:
    case 1:
        o->rot.vx = 256;
        o->rot.vy = 682;
        o->rot.vz = 0;
        o->pos.vx = -512;
        o->pos.vy = 448;
        o->pos.vz = -3036;
        break;
    case -4:
    case 2:
        o->rot.vx = 256;
        o->rot.vy = 1364;
        o->rot.vz = 0;
        o->pos.vx = -512;
        o->pos.vy = 448;
        o->pos.vz = -2012;
        break;
    case -3:
    case 3:
        o->rot.vx = 256;
        o->rot.vy = 2046;
        o->rot.vz = 0;
        o->pos.vx = 0;
        o->pos.vy = 448;
        o->pos.vz = -2012;
        break;
    case -2:
    case 4:
        o->rot.vx = 256;
        o->rot.vy = 2728;
        o->rot.vz = 0;
        o->pos.vx = 512;
        o->pos.vy = 448;
        o->pos.vz = -2012;
        break;
    case -1:
    case 5:
        o->rot.vx = 256;
        o->rot.vy = 3410;
        o->rot.vz = 0;
        o->pos.vx = 512;
        o->pos.vy = 448;
        o->pos.vz = -3036;
        break;
    }
    o->w.panel.label = func_8004F6DC(kind, slot, o);
    return o;
}
