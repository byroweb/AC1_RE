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

void func_8006F8C0(UiObj *o)
{
    EmbRgbW *w = (EmbRgbW *)&o->w;
    u8 c;
    s32 y;

    if (w->chan == 0) {
        c = D_8017E5D8[w->edit->color].r;
        y = 183;
    } else if (w->chan == 1) {
        c = D_8017E5D8[w->edit->color].g;
        y = 191;
    } else {
        c = D_8017E5D8[w->edit->color].b;
        y = 199;
    }
    o->pos.vx = (c >> 3) | 0x60;
    o->pos.vy = y;
    o->pos.vz = 0;
}
