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
extern char D_8004B7C4[];       /* load-sample confirm message */
short SsUtKeyOn(short, short, short, short, short, short, short);
void func_800506EC(s32);
void func_80050804(s32, s32);
void func_8005C2D0(s32, s32, s32, s32, s32);
void func_8005D8D4(char *, void *, s32, s32);
void func_8006A2C4(UiObj *);
void func_8006BA08(UiObj *);
void func_8006AAC4(UiObj *);
void func_8006AB34(UiObj *);
void func_8006ABA4(UiObj *);
void func_8006AE34(UiObj *);

typedef struct EmbSampleWork {  /* UiObj +0x7C */
    u8 pad7c[0x30];
    UiObj *mark[4];             /* +0xAC */
    UiObj *arrow_up;            /* +0xBC */
    UiObj *arrow_down;          /* +0xC0 */
    u8 answer;                  /* +0xC4 */
    u8 page;                    /* +0xC5 0..2 */
    s8 col;                     /* +0xC6 0..3 */
    s8 row;                     /* +0xC7 0..1 */
    u8 padC8[2];
    u8 bank;                    /* +0xCA 0/1 */
} EmbSampleWork;

typedef struct EmbSampleObj {
    u8 pad[0xAC];
    UiObj *mark[4];             /* +0xAC */
    u8 padBC[0xE];
    u8 bank;                    /* +0xCA */
} EmbSampleObj;

void func_8006A3EC(UiObj *o)
{
    EmbSampleObj *e = (EmbSampleObj *)o;
    EmbSampleWork *w;

    func_8006BA08(o);
    func_8006A2C4(o);
    w = (EmbSampleWork *)&o->w;
    if (e->bank == 1) {
        e->mark[2]->visible = 1;
        e->mark[1]->visible = 1;
        e->mark[3]->visible = 0;
        e->mark[0]->visible = 0;
    } else {
        e->mark[2]->visible = 0;
        e->mark[1]->visible = 0;
        e->mark[3]->visible = 1;
        e->mark[0]->visible = 1;
    }
    if (w->page == 0)
        w->arrow_up->visible = 0;
    else
        w->arrow_up->visible = 1;
    if (w->page == 2)
        w->arrow_down->visible = 0;
    else
        w->arrow_down->visible = 1;

    if (D_801A2550.now & 0x2000) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.latch |= 0x2000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.prev & 0x2000))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.prev & 0x2000))
                D_801A2550.delay0 = 12;
        }
        if (w->col != 3)
            w->col++;
    } else if (D_801A2550.now & 0x8000) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.latch |= 0x8000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.prev & 0x8000))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.prev & 0x8000))
                D_801A2550.delay0 = 12;
        }
        if (w->col != 0)
            w->col--;
    } else if (D_801A2550.now & 0x1000) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
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
        if (w->row == 1) {
            w->row--;
        } else if (w->page != 0) {
            o->update = func_8006AAC4;
            w->page--;
        }
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
        if (w->row == 0) {
            w->row++;
        } else if (w->page != 2) {
            o->update = func_8006AB34;
            w->page++;
        }
    } else if (D_801A2550.now & 0x40) {
        D_801A2550.latch |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        func_8005D8D4(D_8004B7C4, &w->answer, 16, 0);
        w->mark[2]->kill = 1;
        w->mark[1]->kill = 1;
        w->mark[3]->kill = 1;
        w->mark[0]->kill = 1;
        w->arrow_up->kill = 1;
        w->arrow_down->kill = 1;
        o->update = func_8006ABA4;
    } else if (D_801A2550.now & 0x20) {
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.latch |= 0x20;
        func_800506EC(14);
        obj_find_by_id(13)->kill = 1;
        func_8005C2D0(0x10, 0x30, 0x11C, 0x90, 13);
        o->update = func_8006AE34;
        w->mark[2]->kill = 1;
        w->mark[1]->kill = 1;
        w->mark[3]->kill = 1;
        w->mark[0]->kill = 1;
        w->arrow_up->kill = 1;
        w->arrow_down->kill = 1;
    } else if (D_801A2550.now & 0x8) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.latch |= 0x8;
        if (w->bank == 0) {
            w->bank++;
            func_80050804(4, w->bank + 8);
        }
    } else if (D_801A2550.now & 0x4) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.latch |= 0x4;
        if (w->bank == 1) {
            w->bank--;
            func_80050804(4, w->bank + 8);
        }
    }
}
