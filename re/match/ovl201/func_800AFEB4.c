#include "ac1/ac1.h"

typedef struct {
    void *src;                  /* +0x7C */
    s32 arg;                    /* +0x80 */
    u8 kind;                    /* +0x84 'M' = music */
    u8 level;                   /* +0x85 */
} WorkVol;

extern u8 D_80048661;
extern u8 D_80048662;
extern s16 D_801A2318;
extern void func_800AFFD4(UiObj *);

UiObj *func_800AFEB4(u8 kind, void *src, s32 arg) {
    UiObj *o;
    WorkVol *w;

    o = ui_obj_alloc(0);
    o->ctor = func_800AFEB4;
    o->update = func_800AFFD4;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    w = (WorkVol *)&o->w;
    w->src = src;
    w->arg = arg;
    w->kind = kind;
    if (kind == 'M') {
        w->level = D_80048661;
    } else {
        w->level = D_80048662;
    }
    if (kind == 'M') {
        func_8005BA2C(168, 44, 80, 16, 7, 0, 0);
    } else {
        func_8005BA2C(168, 68, 80, 16, 7, 0, 0);
    }
    SsUtKeyOn(D_801A2318, 0, 6, 60, 0, 127, 127);
    return o;
}
