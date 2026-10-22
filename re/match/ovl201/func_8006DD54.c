#include "ac1/ac1.h"

typedef struct EmbImage {
    u8 pad[0x8];
    u8 zoom;            /* +0x08 (edit +0x84) */
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

extern u32 D_801A2568;  /* pad: buttons down now */
extern u32 D_801A256C;  /* pad: buttons down last frame */
extern void func_8006F55C(void);
extern void func_8006E924(EmbEdit *e, s32 done);
extern void func_8006E490(EmbEdit *e, s32 done);
extern void func_8006EC38(EmbEdit *e, s32 done);
extern void func_8006EF54(EmbEdit *e, s32 done);

s32 func_8006DD54(EmbEdit *e)
{
    s16 done = 0;
    EmbCursor *c = e->cursor;
    EmbView *v = e->view;
    EmbImage *img = &e->img;

    if ((u16)(c->x - 172) < 128 && (u16)(c->y - 36) < 128) {
        if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) {
            if (e->img.step == 0) {
                e->img.x0 = v->x0 + (c->x - 172) / e->img.zoom;
                e->img.y0 = v->y0 + (c->y - 36) / e->img.zoom;
                e->img.step++;
            } else {
                done = 1;
                func_8006F55C();
            }
        }
        {
            u32 *pad = &D_801A2568;

            if (*pad & 0x20) {
                if (!(D_801A256C & 0x20) && img->step) {
                    img->step = 0;
                    *pad &= ~0x20;
                }
                if (D_801A2568 & 0x20) {
                    img->step = 0;
                    done = 0;
                }
            }
        }
    } else if (D_801A2568 & 0x20) {
        img->step = 0;
        done = 0;
    }
    if (img->step) {
        switch (img->tool) {
        case 1:
            func_8006E924(e, done);
            break;
        case 4:
            func_8006E490(e, done);
            break;
        case 2:
            func_8006EC38(e, done);
            break;
        case 3:
            func_8006EF54(e, done);
            break;
        }
    }
    if (done) {
        img->step = 0;
    }
    return done;
}
