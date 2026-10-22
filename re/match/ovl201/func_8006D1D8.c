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
extern u8 D_800B812C;           /* pen move repeat counter */
extern s32 D_800B8130;          /* d-pad bits of the previous frame */
UiObj *func_8006FE58(s32 dir);
void func_8006DCEC(UiObj *);
void func_8006DD54(UiObj *);
void func_8006DF7C(UiObj *);
void func_8006E0A8(UiObj *);
void func_8006D6E8(UiObj *);

typedef struct EmbCurWork {     /* pen cursor UiObj +0x7C */
    s16 pad7c;
    s16 x;                      /* +0x7E screen x */
    s16 y;                      /* +0x80 screen y */
} EmbCurWork;

typedef struct EmbWork {        /* editor UiObj +0x7C */
    u8 pad7c[8];
    u8 zoom;                    /* +0x84 2, 4 or 8 */
    u8 pad85;
    u8 color;                   /* +0x86 */
    u8 pad87;
    u8 *pixels;                 /* +0x88 */
    u8 tool;                    /* +0x8C 0..6 */
    u8 pad8d;
    u8 tool_step;               /* +0x8E */
    u8 pad8f[9];
    UiObj *cursor;              /* +0x98 */
    UiObj *view;                /* +0x9C */
    UiObj *arrow_y;             /* +0xA0 edge arrow, vertical */
    UiObj *arrow_x;             /* +0xA4 edge arrow, horizontal */
} EmbWork;

void func_8006D1D8(UiObj *o)
{
    EmbWork *w = (EmbWork *)&o->w;
    EmbCurWork *cw = (EmbCurWork *)&w->cursor->w;
    s16 dx, dy;
    u8 cnt;

    if (w->tool != 6) {
        if ((u16)cw->x - 172U >= 128 || (u16)cw->y - 36U >= 128) {
            if (D_801A2550.now & 0x2000)
                dx = 8;
            else if (D_801A2550.now & 0x8000)
                dx = -8;
            else
                dx = 0;
            if (D_801A2550.now & 0x1000)
                dy = -8;
            else if (D_801A2550.now & 0x4000)
                dy = 8;
            else
                dy = 0;
        } else if ((D_801A2550.now & 0xF000) != D_800B8130 || (cnt = D_800B812C) == 0 || w->zoom == 2) {
            D_800B812C = 1;
            if (D_801A2550.now & 0x8000)
                dx = -w->zoom;
            else if (D_801A2550.now & 0x2000)
                dx = w->zoom;
            else
                dx = 0;
            if (D_801A2550.now & 0x1000)
                dy = -w->zoom;
            else if (D_801A2550.now & 0x4000)
                dy = w->zoom;
            else
                dy = 0;
        } else {
            dy = 0;
            D_800B812C = cnt + 1;
            dx = 0;
            if (D_800B812C >= (w->zoom >> 1))
                D_800B812C = 0;
        }
        D_800B8130 = D_801A2550.now & 0xF000;
        if (cw->x < 172 && dx < 0)
            dx = 0;
        else if (cw->x >= 300 && dx > 0)
            dx = 0;
        if (cw->y < 36 && dy < 0)
            dy = 0;
        else if (cw->y >= 164 && dy > 0)
            dy = 0;
        if (cw->x < 172 && (u32)(cw->y + dy - 36) < 128 && dx > 0) {
            cw->x = 172;
            dx = 0;
        } else if (cw->x >= 300 && (u32)(cw->y + dy - 36) < 128 && dx < 0) {
            dx = 0;
            cw->x = 300 - w->zoom;
        }
        if (cw->y < 36 && (u32)(cw->x + dx - 172) < 128 && dy > 0) {
            cw->y = 36;
            dy = 0;
        } else if (cw->y >= 164 && (u32)(cw->x + dx - 172) < 128 && dy < 0) {
            dy = 0;
            cw->y = 164 - w->zoom;
        }
        cw->x += dx;
        cw->y += dy;
        if (cw->x < 172)
            cw->x = 164;
        if (cw->x >= 300)
            cw->x = 303;
        if (cw->y < 36)
            cw->y = 28;
        if (cw->y >= 164)
            cw->y = 167;
        if ((u16)cw->x - 172U < 128 && w->arrow_x != 0) {
            w->arrow_x->kill = 1;
            w->arrow_x = 0;
        }
        if ((u16)cw->y - 36U < 128 && w->arrow_y != 0) {
            w->arrow_y->kill = 1;
            w->arrow_y = 0;
        }
        if (w->arrow_y == 0) {
            if (cw->y < 36)
                w->arrow_y = func_8006FE58(0);
            else if (cw->y >= 164)
                w->arrow_y = func_8006FE58(2);
        }
        if (w->arrow_x == 0) {
            if (cw->x >= 300)
                w->arrow_x = func_8006FE58(1);
            else if (cw->x < 172)
                w->arrow_x = func_8006FE58(3);
        }
    }
    switch (w->tool) {
    case 0:
        func_8006DCEC(o);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        func_8006DD54(o);
        break;
    case 5:
        func_8006DF7C(o);
        break;
    case 6:
        func_8006E0A8(o);
        break;
    }
    func_8006D6E8(o);
}
