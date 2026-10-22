#include "ac1/ac1.h"

typedef struct {
    u8 pad7c[0xC];
    s16 flag;                   /* +0x88 */
    u8 pad8a[2];
    UiObj *owner;               /* +0x8C */
    UiObj *target;              /* +0x90 */
    s16 n;                      /* +0x94 */
    u8 pad96;
    u8 mode;                    /* +0x97 */
} BoxWork;

extern s16 D_801A2318;
extern void func_8005BA2C(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);
void func_800ADD70(UiObj *);

#define W(o) ((BoxWork *)&(o)->w)

UiObj *func_800ADC4C(UiObj *owner, UiObj *target, s16 n, u8 flag, u8 mode) {
    UiObj *o;

    o = ui_obj_alloc(0);
    o->ctor = func_800ADC4C;
    o->update = func_800ADD70;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    if (flag == 0) {
        W(o)->flag = 0;
    } else {
        W(o)->flag = 1;
    }
    W(o)->owner = owner;
    W(o)->n = n;
    W(o)->target = target;
    W(o)->mode = mode;
    target->used = 2;
    func_8005BA2C(0x58, 0x60, 0x90, 0x30, (s16)(n - 1), 0, 0);
    SsUtKeyOn(D_801A2318, 0, 7, 0x3C, 0, 0x7F, 0x7F);
    return o;
}
