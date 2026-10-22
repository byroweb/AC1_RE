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
extern long SquareRoot0(long a);

void func_8006E490(EmbEdit *e, s16 done)
{
    EmbCursor *c = e->cursor;
    s16 dx, dy, d;
    u16 x0, y0;
    s32 cx, cy, t, t2;
    EmbImage *img;

    cx = (c->x - 172) / e->img.zoom;
    cy = (c->y - 36) / e->img.zoom;
    x0 = e->img.x0;
    img = &e->img;
    y0 = e->img.y0;
    dx = x0 - (e->view->x0 + cx);
    dy = y0 - (e->view->y0 + cy);
    t = dx;
    if (t < 0) {
        dx = -t;
    }
    t2 = dy;
    if (t2 < 0) {
        dy = -t2;
    }
    dx = SquareRoot0(dx * dx + dy * dy);
    dy = 0;
    d = 3 - dx * 2;
    if (done) {
        for (; dy <= dx; dy++) {
            func_8006C408(img, (s16)(x0 + dx), (s16)(y0 + dy));
            func_8006C408(img, (s16)(x0 + dx), (s16)(y0 - dy));
            func_8006C408(img, (s16)(x0 - dx), (s16)(y0 + dy));
            func_8006C408(img, (s16)(x0 - dx), (s16)(y0 - dy));
            func_8006C408(img, (s16)(x0 + dy), (s16)(y0 + dx));
            func_8006C408(img, (s16)(x0 + dy), (s16)(y0 - dx));
            func_8006C408(img, (s16)(x0 - dy), (s16)(y0 + dx));
            func_8006C408(img, (s16)(x0 - dy), (s16)(y0 - dx));
            if (d < 0) {
                d += dy * 4 + 6;
            } else {
                d += (dy - dx) * 4 + 10;
                dx--;
            }
        }
    } else {
        for (; dy <= dx; dy++) {
            func_8007053C(img, x0 + dx, y0 + dy, 3, D_800B7E58, 1);
            func_8007053C(img, x0 + dx, y0 - dy, 3, D_800B7E58, 1);
            func_8007053C(img, x0 - dx, y0 + dy, 3, D_800B7E58, 1);
            func_8007053C(img, x0 - dx, y0 - dy, 3, D_800B7E58, 1);
            func_8007053C(img, x0 + dy, y0 + dx, 3, D_800B7E58, 1);
            func_8007053C(img, x0 + dy, y0 - dx, 3, D_800B7E58, 1);
            func_8007053C(img, x0 - dy, y0 + dx, 3, D_800B7E58, 1);
            func_8007053C(img, x0 - dy, y0 - dx, 3, D_800B7E58, 1);
            if (d < 0) {
                d += dy * 4 + 6;
            } else {
                d += (dy - dx) * 4 + 10;
                dx--;
            }
        }
    }
}
