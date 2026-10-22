#include "ac1/ac1.h"

extern char D_8004B758[];       /* APPLY confirm message */
extern char D_8004B770[];       /* CANCEL confirm message */
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
void func_8005BA2C(s32, s32, s32, s32, s32, s32, s32);
void func_8005C2D0(s32, s32, s32, s32, s32);
void func_8005D8D4(char *, void *, s32, s32);
void func_8006BA08(UiObj *);
void func_8006F55C(void);
void func_8006F590(UiObj *);
void func_80069490(UiObj *);
void func_80069528(UiObj *);
void func_8006957C(UiObj *);
void func_8006A14C(UiObj *);
void func_8006AFA0(UiObj *);
void func_8006B080(UiObj *);

typedef struct EmbWork {        /* UiObj +0x7C */
    u8 pad7c[8];
    u8 zoom;                    /* +0x84 */
    u8 zoom_prev;               /* +0x85 */
    u8 color;                   /* +0x86 */
    u8 pad87;
    u32 *pixels;                /* +0x88 */
    u8 pad8c[4];
    u8 cursor;                  /* +0x90 menu item 0..5 */
    u8 zoom_sel;                /* +0x91 */
    u8 pad92[0x32];
    u8 answer[4];               /* +0xC4 */
} EmbWork;

void func_80068FBC(UiObj *o)
{
    EmbWork *w = (EmbWork *)&o->w;
    u32 *p, *end;

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
        if (w->cursor != 0)
            w->cursor--;
        else
            w->cursor = 5;
    } else if (D_801A2550.now & 0x4000) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
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
        if (w->cursor != 5)
            w->cursor++;
        else
            w->cursor = 0;
    }
    if ((D_801A2550.now & 0x20) && !(D_801A2550.prev & 0x20)) {
        D_801A2550.latch |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0x50, 0x32, 0x68, 0x90, 9);
        o->update = func_80069528;
    } else if ((D_801A2550.now & 0x40) && !(D_801A2550.prev & 0x40)) {
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.latch |= 0x40;
        switch (w->cursor) {
        case 0:
            w->zoom_sel = (w->zoom >> 1) - 1;
            if (w->zoom_sel >= 3)
                w->zoom_sel = 2;
            w->zoom_prev = w->zoom;
            func_8005BA2C(0xBC, 0x32, 0x30, 0x30, 11, 0, 0);
            SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
            o->update = func_8006957C;
            break;
        case 1:
            func_8006F590(o);
            o->update = func_80069490;
            break;
        case 2:
            p = w->pixels;
            end = p + 512;
            func_8006F55C();
            while (p < end)
                *p++ = 0;
            o->update = func_80069490;
            break;
        case 3:
            func_8005BA2C(0x10, 0x30, 0x11C, 0x90, 13, 1, 0);
            SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
            o->update = func_8006A14C;
            break;
        case 4:
            func_8005D8D4(D_8004B758, w->answer, 16, 1);
            o->update = func_8006B080;
            break;
        case 5:
            func_8005D8D4(D_8004B770, w->answer, 16, 1);
            o->update = func_8006AFA0;
            break;
        }
    }
    func_8006BA08(o);
}
