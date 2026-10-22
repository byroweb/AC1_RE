#include "ac1/ac1.h"
/* --- emb.h (shared by 0x8006E490/E924/EC38/EF54/F2CC) --- */
/* local views of the emblem editor work (EmbEdit is the UiObj; img is its work area at +0x7C) */
typedef struct EmbImage {
    u8 pad[0x8];
    u8 zoom;            /* +0x08 (edit +0x84) 2/4/8 */
    u8 pad9;
    u8 color;           /* +0x0A pen colour (edit +0x86) */
    u8 padB;
    u8 *pixels;         /* +0x0C 4bpp 64x64, 32 bytes per row */
    u8 tool;            /* +0x10 (edit +0x8C) */
    u8 pad11;
    u8 step;            /* +0x12 (edit +0x8E) tool step */
    u8 pad13[3];
    s16 x0;             /* +0x16 (edit +0x92) start pixel */
    s16 y0;             /* +0x18 (edit +0x94) */
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
    EmbCursor *cursor;  /* +0x98 */
    EmbView *view;      /* +0x9C */
} EmbEdit;
/* --- end emb.h --- */

extern u8 D_800B7E58[];
extern void func_8006C408(EmbImage *img, s32 x, s32 y);
extern void func_8007053C(EmbImage *img, s16 x, s16 y, s16 c, void *a, s32 b);

void func_8006EC38(EmbEdit *e, s16 done)
{
    EmbCursor *c = e->cursor;
    s16 xa, xb, ya, yb, i;
    s32 cx, cy;
    EmbImage *img;

    cx = (c->x - 172) / e->img.zoom;
    cy = (c->y - 36) / e->img.zoom;
    img = &e->img;
    yb = e->img.y0;
    xb = e->img.x0;
    xa = e->view->x0 + cx;
    ya = e->view->y0 + cy;
    if (xb < xa) {
        i = xa;
        xa = xb;
        xb = i;
    }
    if (ya > yb) {
        i = ya;
        ya = yb;
        yb = i;
    }
    if (done) {
        for (i = xa; i <= xb; i++) {
            func_8006C408(img, i, ya);
            func_8006C408(img, i, yb);
        }
        for (i = ya + 1; i <= yb - 1; i++) {
            func_8006C408(img, xa, i);
            func_8006C408(img, xb, i);
        }
    } else {
        for (i = xa; i <= xb; i++) {
            func_8007053C(img, i, ya, 3, D_800B7E58, 1);
            func_8007053C(img, i, yb, 3, D_800B7E58, 1);
        }
        for (i = ya + 1; i <= yb - 1; i++) {
            func_8007053C(img, xa, i, 3, D_800B7E58, 1);
            func_8007053C(img, xb, i, 3, D_800B7E58, 1);
        }
    }
}
