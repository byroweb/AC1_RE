#include "ac1/ac1.h"

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
u8 func_8006C4C0(void *, s16, s16);
void func_8006F55C(void);

typedef struct EmbCurWork {     /* pen cursor UiObj +0x7C */
    s16 pad7c;
    s16 x;                      /* +0x7E screen x */
    s16 y;                      /* +0x80 screen y */
} EmbCurWork;

typedef struct EmbViewWork {    /* canvas view UiObj +0x7C */
    s16 x0;                     /* +0x7C first canvas column shown */
    s16 y0;                     /* +0x7E first canvas row shown */
} EmbViewWork;

typedef struct EmbWork {        /* editor UiObj +0x7C */
    u8 pad7c[8];
    u8 zoom;                    /* +0x84 2, 4 or 8 */
    u8 pad85;
    u8 color;                   /* +0x86 pen palette index */
    u8 pad87;
    u8 *pixels;                 /* +0x88 */
    u8 tool;                    /* +0x8C 0..6 */
    u8 pad8d;
    u8 tool_step;               /* +0x8E */
    u8 pad8f[9];
    UiObj *cursor;              /* +0x98 */
    UiObj *view;                /* +0x9C */
} EmbWork;

void func_8006D6E8(UiObj *o)
{
    EmbWork *w = (EmbWork *)&o->w;
    EmbCurWork *cw = (EmbCurWork *)&w->cursor->w;
    EmbViewWork *vw = (EmbViewWork *)&w->view->w;
    s16 px, py;

    if ((D_801A2550.now & 0x40)
        && ((u16)(cw->x - 172) >= 128 || (u16)(cw->y - 36) >= 128)
        && w->zoom != 2) {
        if (cw->x < 172) {
            if (vw->x0 != 0)
                vw->x0 -= 2;
        } else if (cw->x >= 300) {
            vw->x0 += 2;
            if (w->zoom == 4) {
                if (vw->x0 > 32)
                    vw->x0 = 32;
            } else {
                if (vw->x0 > 48)
                    vw->x0 = 48;
            }
        }
        if (cw->y < 36) {
            if (vw->y0 != 0)
                vw->y0 -= 2;
        } else if (cw->y >= 164) {
            vw->y0 += 2;
            if (w->zoom == 4) {
                if (vw->y0 > 32)
                    vw->y0 = 32;
            } else {
                if (vw->y0 > 48)
                    vw->y0 = 48;
            }
        }
    }
    px = vw->x0 + (cw->x - 172) / w->zoom;
    py = vw->y0 + (cw->y - 36) / w->zoom;
    if ((D_801A2550.now & 0x10) && !(D_801A2550.prev & 0x10)
        && (u16)(cw->x - 172) < 128 && (u16)(cw->y - 36) < 128) {
        w->color = func_8006C4C0(w, px, py);
    }
    if (D_801A2550.now & 0x2) {
        D_801A2550.latch |= 0x2;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.prev & 0x2))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.prev & 0x2))
                D_801A2550.delay0 = 12;
        }
        if (++w->color == 16)
            w->color = 0;
    } else if (D_801A2550.now & 0x1) {
        D_801A2550.latch |= 0x1;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.prev & 0x1))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.prev & 0x1))
                D_801A2550.delay0 = 12;
        }
        w->color = w->color != 0 ? w->color - 1 : 15;
    }
    if ((D_801A2550.now & 0x100) && !(D_801A2550.prev & 0x100)
        && (D_801A2550.now & 0xC) != 0xC) {
        if (w->zoom == 2) {
            vw->x0 = (px - 16) & ~1;
            vw->y0 = (py - 16) & ~1;
            w->zoom = 4;
            if (vw->x0 > 32)
                vw->x0 = 32;
            if (vw->y0 > 32)
                vw->y0 = 32;
        } else if (w->zoom == 4) {
            vw->x0 = (px - 8) & ~1;
            vw->y0 = (py - 8) & ~1;
            w->zoom = 8;
            if (vw->x0 > 48)
                vw->x0 = 48;
            if (vw->y0 > 48)
                vw->y0 = 48;
        } else {
            w->zoom = 2;
            vw->x0 = 0;
            vw->y0 = 0;
        }
        if (vw->x0 < 0)
            vw->x0 = 0;
        if (vw->y0 < 0)
            vw->y0 = 0;
        if ((u16)cw->x - 172U < 128 && (u16)cw->y - 36U < 128) {
            cw->x = (px - vw->x0) * w->zoom + 172;
            cw->y = (py - vw->y0) * w->zoom + 36;
        }
    }
    if ((D_801A2550.now & 0x8) && !(D_801A2550.prev & 0x8)) {
        w->tool++;
        w->tool_step = 0;
        if (w->tool >= 7)
            w->tool = 0;
        if (w->tool == 0)
            func_8006F55C();
    } else if ((D_801A2550.now & 0x4) && !(D_801A2550.prev & 0x4)) {
        w->tool = w->tool != 0 ? w->tool - 1 : 6;
        w->tool_step = 0;
        if (w->tool == 0)
            func_8006F55C();
    }
}
