#include "ac1/ac1.h"
/* ovl201 0x80083894 - spawn the grid cursor over the column / row bytes (func_80083958 moves it) */
typedef struct CurWork {
    u8 *col;                    /* +0x7C */
    u8 *row;                    /* +0x80 */
} CurWork;
extern u8 D_800B8550[];
void func_80083958(UiObj *o);

void func_80083894(u8 *col, u8 *row, s16 kind)
{
    UiObj *o;
    CurWork *w;
    s32 x;
    s32 y;

    o = ui_obj_alloc(kind);
    w = (CurWork *)&o->w;
    o->ctor = func_80083894;
    o->update = func_80083958;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 11;
    o->unk14 = 1;
    o->res = D_800B8550;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    w->col = col;
    w->row = row;
    y = *w->row * 16 + 4;
    x = *w->col * 16 + 8;
    o->pos.vy = y;
    o->pos.vz = 0;
    o->pos.vx = x;
}
