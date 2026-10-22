#include "ac1/ac1.h"
/* ovl201 0x800695E8 - emblem editor zoom picker (sel +0x91: x2/x4/x8):
 * up/down re-centres the view on the pen cursor at the new zoom, X restores
 * the previous zoom and closes (func_80069C80), O keeps it (func_80069490). */

extern s16 D_801A2318;
typedef struct PadRepeat {      /* 0x801A2550 */
    s32 latch;                  /* +0x00 */
    s16 delay;                  /* +0x04 */
    s16 delay0;                 /* +0x06 */
    u8 pad08[0x10];
    s32 now;                    /* +0x18 */
    s32 prev;                   /* +0x1C */
} PadRepeat;
extern PadRepeat D_801A2550;
extern s32 D_801A2574;
short SsUtKeyOn(short, short, short, short, short, short, short);
void func_800506EC(s32);
void func_8005C2D0(s32, s32, s32, s32, s32);
void func_8006BA08(UiObj *);
void func_80069490(UiObj *);
void func_80069C80(UiObj *);

typedef struct EmbWork {        /* UiObj +0x7C */
    u8 pad7c[8];
    u8 zoom;                    /* +0x84 */
    u8 zoom_prev;               /* +0x85 */
    u8 color;                   /* +0x86 */
    u8 pad87;
    u32 *pixels;                /* +0x88 */
    u8 pad8c[4];
    u8 cursor;                  /* +0x90 */
    u8 zoom_sel;                /* +0x91 */
    u8 pad92[6];
    UiObj *pen;                 /* +0x98 */
    UiObj *view;                /* +0x9C */
} EmbWork;

typedef struct {                /* pen cursor UiObj +0x7C */
    s16 pad;
    s16 x;                      /* +0x7E */
    s16 y;                      /* +0x80 */
} PenWork;

typedef struct {                /* canvas view UiObj +0x7C */
    s16 sx;                     /* +0x7C scroll x */
    s16 sy;                     /* +0x7E scroll y */
} ViewWork;

#define RECENTRE()                                                      \
    if (w->zoom_sel == 1) {                                             \
        v->sx = (x - 16) & ~1;                                          \
        v->sy = (y - 16) & ~1;                                          \
        w->zoom = 4;                                                    \
        if (v->sx > 32)                                                 \
            v->sx = 32;                                                 \
        if (v->sy > 32)                                                 \
            v->sy = 32;                                                 \
    } else if (w->zoom_sel == 2) {                                      \
        v->sx = (x - 8) & ~1;                                           \
        v->sy = (y - 8) & ~1;                                           \
        w->zoom = 8;                                                    \
        if (v->sx > 48)                                                 \
            v->sx = 48;                                                 \
        if (v->sy > 48)                                                 \
            v->sy = 48;                                                 \
    } else {                                                            \
        w->zoom = 2;                                                    \
        v->sx = 0;                                                      \
        v->sy = 0;                                                      \
    }                                                                   \
    if (v->sx < 0)                                                      \
        v->sx = 0;                                                      \
    if (v->sy < 0)                                                      \
        v->sy = 0;                                                      \
    if ((u32)((u16)c->x - 172) < 128 && (u32)((u16)c->y - 36) < 128) {  \
        c->x = (x - v->sx) * w->zoom + 172;                             \
        c->y = (y - v->sy) * w->zoom + 36;                              \
    }

void func_800695E8(UiObj *o)
{
    EmbWork *w = (EmbWork *)&o->w;
    PenWork *c = (PenWork *)&w->pen->w;
    ViewWork *v = (ViewWork *)&w->view->w;
    s16 x, y;

    if (D_801A2550.now & 0x1000) {
        D_801A2550.latch |= 0x1000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.prev & 0x1000))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.prev & 0x1000))
                D_801A2550.delay0 = 12;
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->zoom_sel != 0)
            w->zoom_sel--;
        else
            w->zoom_sel = 2;
    } else if (D_801A2550.now & 0x4000) {
        D_801A2550.latch |= 0x4000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.prev & 0x4000))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.prev & 0x4000))
                D_801A2550.delay0 = 12;
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->zoom_sel != 2)
            w->zoom_sel++;
        else
            w->zoom_sel = 0;
    }
    x = v->sx + (c->x - 172) / w->zoom;
    y = v->sy + (c->y - 36) / w->zoom;
    if (D_801A2550.now & 0x20) {
        D_801A2550.latch = D_801A2550.now & 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        w->zoom = w->zoom_prev;
        if (w->zoom_prev == 2)
            w->zoom_sel = 0;
        else if (w->zoom_prev == 4)
            w->zoom_sel = 1;
        else
            w->zoom_sel = 2;
        RECENTRE();
        func_800506EC(12);
        obj_find_by_id(11)->kill = 1;
        func_8005C2D0(0xBC, 0x32, 0x40, 0x40, 11);
        o->update = func_80069C80;
    } else if (D_801A2550.now & 0x5020) {
        RECENTRE();
    } else if ((D_801A2550.now & 0x40) && !(D_801A2550.prev & 0x40)) {
        D_801A2550.latch |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        func_800506EC(12);
        obj_find_by_id(11)->kill = 1;
        func_8005C2D0(0xBC, 0x32, 0x40, 0x40, 11);
        o->update = func_80069490;
    }
    func_8006BA08(o);
}
