#include "ac1/ac1.h"
/* ovl201 0x80051DF0 - message box object: opens a frame (func_8005BA2C), shows text, waits for a button */
typedef struct MsgWork {
    UiObj *owner;               /* +0x7C */
    void *next;                 /* +0x80 owner's update on close */
    u8 *text;                   /* +0x84 */
    s16 id;                     /* +0x88 frame object id + 1 */
    u8 pad8a[2];
    UiObj *label;               /* +0x8C */
    s16 x, y, w, h;             /* +0x90 */
} MsgWork;
void func_80051F24(UiObj *o);
void func_8005BA2C(s16 x, s16 y, s16 w, s16 h, s32 id, s32 a, s32 b);

UiObj *func_80051DF0(UiObj *owner, void *next, u8 *text, s16 id, s16 x, s16 y, s16 w, s16 h)
{
    UiObj *o;
    MsgWork *m;

    o = ui_obj_alloc(0);
    o->ctor = func_80051DF0;
    o->update = func_80051F24;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    m = (MsgWork *)&o->w;
    m->owner = owner;
    m->next = next;
    m->text = text;
    m->id = id;
    m->y = y;
    m->w = w;
    m->h = h;
    m->x = x;
    func_8005BA2C(x, y, w, h, (s16)(id - 1), 1, 0);
    return o;
}
