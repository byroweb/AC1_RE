#include "ac1/ac1.h"

typedef struct EmbEdit {
    u8 pad[0x86];
    u8 color;                   /* +0x86 pen palette index */
} EmbEdit;

typedef struct EmbRgbW {
    EmbEdit *edit;              /* +0x7C */
    s16 chan;                   /* +0x80 0 r, 1 g, 2 b */
} EmbRgbW;

typedef struct Rgba {
    u8 r, g, b, a;
} Rgba;

extern Rgba D_8017E5D8[16];
extern u8 D_800B815C[];
extern void func_8006F8C0(UiObj *o);

UiObj *func_8006F79C(EmbEdit *edit, s16 chan)
{
    UiObj *o;
    EmbRgbW *w;
    s32 x;
    s32 y;

    o = ui_obj_alloc(4);
    o->ctor = func_8006F79C;
    o->update = func_8006F8C0;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->unk14 = 3;
    o->res = D_800B815C;
    o->light = 0;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    w = (EmbRgbW *)&o->w;
    w->edit = edit;
    w->chan = chan;
    if (chan == 0) {
        x = (D_8017E5D8[edit->color].r >> 3) | 0x60;
        y = 183;
    } else if (chan == 1) {
        x = (D_8017E5D8[edit->color].g >> 3) | 0x60;
        y = 191;
    } else {
        x = (D_8017E5D8[edit->color].b >> 3) | 0x60;
        y = 199;
    }
    o->pos.vx = x;
    o->pos.vy = y;
    o->pos.vz = 0;
    return o;
}
