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
extern s16 D_801A2318;

typedef struct { u8 r, g, b, a; } EmbRgb;
extern EmbRgb D_8017E5D8[16];   /* editor palette */
extern EmbRgb D_8017E618;       /* colour before editing (undo) */
extern EmbRgb D_800B7E48;       /* background (colour 0) */

typedef struct EmbW {           /* editor UiObj +0x7C */
    u8 pad[0xA];
    u8 color;                   /* +0x0A (obj +0x86) */
    u8 padB[8];
    s8 fade;                    /* +0x13 (obj +0x8F) */
    u8 pad14[6];
    u8 chan;                    /* +0x1A (obj +0x96) 0 r, 1 g, 2 b */
} EmbW;

extern void func_8006BA08(UiObj *o);
extern void func_8006CC78(UiObj *o);
extern void func_8006F5E8(void);
extern void func_8006F670(void);
extern void func_8006F79C(void);
extern void func_80068C08(UiObj *o);
short SsUtKeyOn(short, short, short, short, short, short, short);

void func_8006B250(UiObj *o)
{
    EmbW *w = (EmbW *)&o->w;
    u8 *p;

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
        D_8017E5D8[w->color] = D_8017E618;
        if (w->color == 0) {
            D_800B7E48.r = D_8017E618.r;
            D_800B7E48.g = D_8017E618.g;
            D_800B7E48.b = D_8017E618.b;
            w->fade = -1;
        }
        if (++w->color == 16)
            w->color = 0;
        D_8017E618 = D_8017E5D8[w->color];
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
        D_8017E5D8[w->color] = D_8017E618;
        if (w->color == 0) {
            D_800B7E48.r = D_8017E618.r;
            D_800B7E48.g = D_8017E618.g;
            D_800B7E48.b = D_8017E618.b;
            w->fade = -1;
        }
        if (w->color == 0)
            w->color = 15;
        else
            w->color--;
        D_8017E618 = D_8017E5D8[w->color];
    }
    if ((D_801A2550.now & 0x1000) && !(D_801A2550.prev & 0x1000)) {
        if (w->chan == 0)
            w->chan = 2;
        else
            w->chan--;
    } else if ((D_801A2550.now & 0x4000) && !(D_801A2550.prev & 0x4000)) {
        if (w->chan == 2)
            w->chan = 0;
        else
            w->chan++;
    }
    if (w->chan == 0) {
        p = &D_8017E5D8[w->color].r;
    } else if (w->chan == 1) {
        p = &D_8017E5D8[w->color].g;
    } else {
        p = &D_8017E5D8[w->color].b;
    }
    if ((D_801A2550.now & 0x2000) && *p != 0xF8) {
        D_801A2550.latch |= 0x2000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 0;
            if (!(D_801A2550.prev & 0x2000))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 0;
            if (!(D_801A2550.prev & 0x2000))
                D_801A2550.delay0 = 12;
        }
        *p += 8;
        if (w->color == 0) {
            D_800B7E48.r = D_8017E5D8[0].r;
            D_800B7E48.g = D_8017E5D8[0].g;
            D_800B7E48.b = D_8017E5D8[0].b;
            w->fade = -1;
        }
    } else if ((D_801A2550.now & 0x8000) && *p != 0) {
        D_801A2550.latch |= 0x8000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 0;
            if (!(D_801A2550.prev & 0x8000))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 0;
            if (!(D_801A2550.prev & 0x8000))
                D_801A2550.delay0 = 12;
        }
        *p -= 8;
        if (w->color == 0) {
            D_800B7E48.r = D_8017E5D8[0].r;
            D_800B7E48.g = D_8017E5D8[0].g;
            D_800B7E48.b = D_8017E5D8[0].b;
            w->fade = -1;
        }
    }
    if ((D_801A2550.now & 0x40) && !(D_801A2550.prev & 0x40)) {
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        D_8017E618 = D_8017E5D8[w->color];
    } else if ((D_801A2550.now & 0x20) && !(D_801A2550.prev & 0x20)) {
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        D_8017E5D8[w->color] = D_8017E618;
        if (w->color == 0) {
            D_800B7E48.r = D_8017E5D8[0].r;
            D_800B7E48.g = D_8017E5D8[0].g;
            D_800B7E48.b = D_8017E5D8[0].b;
        }
    }
    func_8006BA08(o);
    if (((D_801A2550.now & 0x80) && !(D_801A2550.prev & 0x80))
        || ((D_801A2550.now & 0x40) && !(D_801A2550.prev & 0x40))
        || ((D_801A2550.now & 0x20) && !(D_801A2550.prev & 0x20))) {
        obj_find_by_handle(func_8006F5E8)->kill = 1;
        obj_find_by_handle(func_8006F670)->kill = 1;
        obj_find_by_handle(func_8006F79C)->kill = 1;
        obj_find_by_handle(func_8006F79C)->kill = 1;
        obj_find_by_handle(func_8006F79C)->kill = 1;
        D_8017E5D8[w->color] = D_8017E618;
        o->update = func_80068C08;
        D_801A2550.latch |= D_801A2550.now;
    } else {
        func_8006CC78(o);
    }
}
