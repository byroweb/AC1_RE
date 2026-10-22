#include "ac1/ac1.h"
/* ovl201 0x800804F0 - build the invisible list object of the emblem screen
 * (update func_800807CC): cursor and first line from the D_80039CA2 variant,
 * the up / down arrows (the up one hidden for variants below 8), the visible
 * lines (func_80080EB0) and func_80080618. */

typedef struct {
    void *owner;                /* +0x7C */
    s32 arg;                    /* +0x80 */
    UiObj *up;                  /* +0x84 */
    UiObj *down;                /* +0x88 */
    u8 cursor;                  /* +0x8C */
    u8 first;                   /* +0x8D */
    u8 cur2;                    /* +0x8E */
} ListWork;

extern u8 D_80039CA2;
extern u8 D_800B91AC[];
extern u8 D_800B91B8[];
extern UiObj *func_8005CAE0(void *e, s32 x, s16 y, s32 kind);
extern void func_800807CC(UiObj *o);
extern void func_80080EB0(UiObj *o);
extern void func_80080618(UiObj *o);

UiObj *func_800804F0(void *owner, s32 arg)
{
    UiObj *o;
    ListWork *w;
    u8 v;

    o = ui_obj_alloc(0);
    o->visible = 0;
    o->attached = 0;
    o->has_xform = 0;
    o->ctor = func_800804F0;
    o->update = func_800807CC;
    w = (ListWork *)&o->w;
    w->owner = owner;
    o->destroy = ui_obj_nop;
    o->parent = 0;
    w->arg = arg;
    v = D_80039CA2;
    w->cursor = v;
    if (v < 7) {
        w->first = 0;
    } else {
        w->first = v - 6;
    }
    w->cur2 = D_80039CA2;
    w->up = func_8005CAE0(D_800B91AC, 4, 40, 2);
    w->up->light = 11;
    if (D_80039CA2 < 8) {
        w->up->visible = 0;
    }
    w->down = func_8005CAE0(D_800B91B8, 4, 192, 2);
    w->down->light = 11;
    func_80080EB0(o);
    func_80080618(o);
    return o;
}
