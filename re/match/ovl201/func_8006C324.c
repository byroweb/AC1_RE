#include "ac1/ac1.h"

typedef struct EmbCursor {     /* UiObj view: the pen cursor */
    u8 pad[0x7E];
    s16 x;                      /* +0x7E screen x */
    s16 y;                      /* +0x80 screen y */
} EmbCursor;

typedef struct EmbView {
    u8 pad[0x7C];
    s16 x0;                     /* +0x7C first canvas column on screen */
    s16 y0;                     /* +0x7E first canvas row on screen */
} EmbView;

typedef struct EmbEdit {
    u8 pad[0x84];
    u8 zoom;                    /* +0x84 */
    u8 pad85;
    u8 color;                   /* +0x86 palette index of the pen */
    u8 pad87;
    u8 *pixels;                 /* +0x88 4bpp, 32 bytes per row */
    u8 pad8c[0xC];
    EmbCursor *cursor;          /* +0x98 */
    EmbView *view;              /* +0x9C */
} EmbEdit;

void func_8006C324(EmbEdit *o)
{
    s16 x, y;
    u8 *p;

    x = o->view->x0 + (o->cursor->x - 172) / o->zoom;
    y = o->view->y0 + (o->cursor->y - 36) / o->zoom;
    if (x & 1) {
        p = &o->pixels[x / 2 + y * 32];
        *p &= 0xF;
        *p |= o->color << 4;
    } else {
        p = &o->pixels[x / 2 + y * 32];
        *p &= 0xF0;
        *p |= o->color & 0xF;
    }
}
