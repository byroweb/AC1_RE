#include "ac1/ac1.h"

typedef struct EmbImage {
    u8 pad[0x8];
    u8 zoom;            /* +0x08 (edit +0x84) */
    u8 pad9;
    u8 color;           /* +0x0A pen colour (edit +0x86) */
    u8 padB;
    u8 *pixels;         /* +0x0C 4bpp 64x64, 32 bytes per row */
} EmbImage;

typedef struct EmbCursor {
    u8 pad[0x7E];
    s16 x;              /* +0x7E */
    s16 y;              /* +0x80 */
} EmbCursor;

typedef struct EmbView {
    u8 pad[0x7C];
    s16 x0;             /* +0x7C */
    s16 y0;             /* +0x7E */
} EmbView;

typedef struct EmbEdit {
    u8 hdr[0x7C];
    EmbImage img;       /* +0x7C */
    u8 pad8c[0xC];
    EmbCursor *cursor;  /* +0x98 */
    EmbView *view;      /* +0x9C */
} EmbEdit;

typedef struct EmbToolW {
    EmbEdit *edit;      /* +0x7C */
} EmbToolW;

extern u8 D_800B7E54[];
extern void func_8007053C(EmbImage *img, s16 x, s16 y, s16 c, void *a, s32 b);

void func_8007046C(UiObj *o)
{
    EmbEdit *e = ((EmbToolW *)&o->w)->edit;
    EmbCursor *c = e->cursor;
    EmbView *v = e->view;
    EmbImage *img = &e->img;
    s16 x, y;

    if (c->x >= 172 && c->y >= 36) {
        x = v->x0 + (c->x - 172) / img->zoom;
        y = v->y0 + (c->y - 36) / img->zoom;
        if (x < 64 && y < 64 && x >= 0 && y >= 0) {
            func_8007053C(img, x, y, o->unk14, D_800B7E54, 0);
        }
    }
}
