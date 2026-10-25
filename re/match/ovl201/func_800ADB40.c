#include "ac1/ac1.h"

/* Work view of a scrolling row: pos.vy = y + *src * 0x18 (func_800ADC20). */
typedef struct {
    u8 *src;                    /* +0x7C byte that selects the row */
    s16 x;                      /* +0x80 */
    s16 y;                      /* +0x82 */
    s16 arg;                    /* +0x84 */
} RowWork;

void func_800ADC20(UiObj *);
extern u8 D_800BDD40[];

void func_800ADB40(u8 *src, s16 x, s16 y, s16 arg) {
    UiObj *o;
    RowWork *w;
    s32 px, py;

    o = ui_obj_alloc(arg);
    w = (RowWork *)&o->w;
    o->ctor = func_800ADB40;
    o->update = func_800ADC20;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 0x15;
    o->unk14 = 1;
    o->res = D_800BDD40;
    w->x = x;
    px = x;
    py = y;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    do { w->src = src; } while (0);
    w->y = y;
    w->arg = arg;
    o->pos.vx = px;
    o->pos.vy = py;
    o->pos.vz = 0;
}
