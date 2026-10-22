#include "ac1/ac1.h"
/* ovl201 0x800800A4 - build the 3D preview of the part equipped in category
 * cat (model given), scaled and placed from its entry in the D_800BCAC0
 * table set. */

typedef struct {
    s16 lim;
    s16 y;                      /* +0x02 */
    s16 z;                      /* +0x04 */
    u8 pad06[6];
    s32 scale;                  /* +0x0C */
} PartEntry;

typedef struct {
    PartEntry *e;
    u8 pad[12];
} PartSet;

typedef struct {
    s16 kind;
    s16 model;
} ModelRes;

extern PartSet D_800BCAC0[];
extern u8 D_80031A58[];
extern void func_800804DC(UiObj *o);

void func_800800A4(s16 model, s16 cat, s16 set)
{
    UiObj *o;

    o = ui_obj_alloc(4);
    o->ctor = func_800800A4;
    o->update = func_800804DC;
    o->destroy = ui_obj_nop;
    o->light = 18;
    o->render_type = 0;
    ((ModelRes *)&o->res)->kind = 0;
    ((ModelRes *)&o->res)->model = model;
    o->scaled = 1;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->scale.vx = D_800BCAC0[set].e[D_80031A58[cat]].scale;
    o->scale.vy = D_800BCAC0[set].e[D_80031A58[cat]].scale;
    o->scale.vz = D_800BCAC0[set].e[D_80031A58[cat]].scale;
    o->pos.vx = 0;
    o->pos.vy = D_800BCAC0[set].e[D_80031A58[cat]].y + 206;
    o->pos.vz = D_800BCAC0[set].e[D_80031A58[cat]].z;
}
