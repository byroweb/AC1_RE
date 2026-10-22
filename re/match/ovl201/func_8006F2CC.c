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

extern u8 func_8006C4C0(EmbImage *img, s32 x, s32 y);
extern void func_8006C408(EmbImage *img, s32 x, s32 y);

u8 *func_8006F2CC(EmbEdit *e, u8 *sp, u8 old)
{
    EmbImage *img;
    s16 l, r, y, x;
    u8 flag;

    sp -= 2;
    img = &e->img;
    l = sp[0];
    y = sp[1];
    r = l;
    while (l > 0 && func_8006C4C0(img, (s16)(l - 1), y) == old) {
        l--;
    }
    while (r < 63 && func_8006C4C0(img, (s16)(r + 1), y) == old) {
        r++;
    }
    for (x = l; x <= r; x++) {
        func_8006C408(img, x, y);
    }
    if (y != 0) {
        for (x = r, flag = 0; x >= l; x--) {
            if (func_8006C4C0(img, x, (s16)(y - 1)) == old && !flag) {
                sp[0] = x;
                sp[1] = y - 1;
                sp += 2;
                flag = 1;
            } else {
                flag = 0;
            }
        }
    }
    if (y != 63) {
        for (x = r, flag = 0; x >= l; x--) {
            if (func_8006C4C0(img, x, (s16)(y + 1)) == old && !flag) {
                sp[0] = x;
                sp[1] = y + 1;
                sp += 2;
                flag = 1;
            } else {
                flag = 0;
            }
        }
    }
    return sp;
}
