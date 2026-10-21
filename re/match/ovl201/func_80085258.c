#include "ac1/ac1.h"
/* ovl201 0x80085258 - spawn model object i of the table at D_800BCB50 (scale and z from
 * its 16-byte entry). The table pointer is a struct member: retail keeps its load
 * after the object stores. */
typedef struct ModelEnt {
    u8 unk00[4];
    s16 z;                      /* +0x04 */
    u8 unk06[6];
    s32 scale;                  /* +0x0C */
} ModelEnt;
typedef struct ModelTab {
    ModelEnt *ent;
} ModelTab;
extern ModelTab D_800BCB50;
void func_8008530C(UiObj *o);



void func_80085258(s16 i)
{
    UiObj *o;
    ModelEnt *e;
    s32 s;

    o = ui_obj_alloc(6);
    o->ctor = func_80085258;
    o->update = func_8008530C;
    o->destroy = ui_obj_nop;
    ((u16 *)&o->res)[1] = 13;
    o->light = 18;
    o->render_type = 0;
    ((u16 *)&o->res)[0] = 0;
    o->scaled = 1;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    e = (ModelEnt *)((i << 4) + (s32)D_800BCB50.ent);
    s = e->scale;
    o->pos.vx = 0;
    o->pos.vy = 0;
    o->scale.vx = s;
    o->scale.vy = s;
    o->scale.vz = s;
    o->pos.vz = e->z;
}
