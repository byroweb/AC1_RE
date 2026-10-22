#include "ac1/ac1.h"

typedef struct PadRep {
    u32 latch;                  /* +0x00 0x801A2550 */
    s16 delay;                  /* +0x04 pad 1 first repeat delay */
    s16 rate;                   /* +0x06 pad 1 repeat rate */
    u8 pad08[4];
    s16 delay2;                 /* +0x0C pad 2 */
    s16 rate2;                  /* +0x0E pad 2 */
    u8 pad10[8];
    u32 press;                  /* +0x18 0x801A2568 */
    u32 held;                   /* +0x1C 0x801A256C */
} PadRep;

typedef struct { u8 b[160]; } Blk160;
typedef struct { u8 b[36]; } Blk36;
typedef struct { s32 w[12]; } Blk48;
typedef struct { u8 b[7]; } Blk7;

extern PadRep D_801A2550_s;
extern u32 D_801A2550;
extern u32 D_801A2568;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern s8 D_801A2570;
extern Blk48 D_80048630;
extern Blk48 D_80048538[];
extern s16 D_800415CC[12];
extern Blk160 D_80042AA8;
extern Blk160 D_80040E60[];
extern Blk36 D_80031BD4;
extern Blk36 D_800419B8[];
extern Blk7 D_80039D20;
extern Blk7 D_80040E38[];
extern u8 D_80048663[];
extern u8 D_80039CAE;
extern u8 D_800BDD4C[];
extern u8 D_800BDD58[];
extern u8 D_8004D3E8[];
extern void func_800AE838(void);
extern void func_800AE184(void);
extern void func_80083A58(void);
extern void func_800AAC20(u8, UiObj *);
extern void func_8005BA2C(s32, s32, s32, s32, s32, s32, s32);
extern UiObj *func_8005CAE0(void *, s32, s32, s32);
extern void func_800ADC4C(void *, u8 *, s32, s32, s32);
extern void func_800AA520(UiObj *);
extern void func_800AB584(UiObj *);
extern void func_800AB938(UiObj *);
extern void func_800AD868(UiObj *);

void func_800A9AFC(UiObj *o)
{
    UiWork *w = &o->w;

    if (D_801A2550_s.press & 0x10001000) {
        if (D_801A2550_s.press & 0x1000) {
            D_801A2550_s.latch |= 0x1000;
            if (D_801A2574 != 0) {
                D_801A2550_s.delay = 6 / D_801A2574;
                if (!(D_801A2550_s.held & 0x1000)) {
                    D_801A2550_s.rate = 12 / D_801A2574;
                }
            } else {
                D_801A2550_s.delay = 6;
                if (!(D_801A2550_s.held & 0x1000)) {
                    D_801A2550_s.rate = 12;
                }
            }
        }
        if (D_801A2550_s.press & 0x10000000) {
            D_801A2550_s.latch |= 0x10000000;
            if (D_801A2574 != 0) {
                D_801A2550_s.delay2 = 6 / D_801A2574;
                if (!(D_801A2550_s.held & 0x10000000)) {
                    D_801A2550_s.rate2 = 12 / D_801A2574;
                }
            } else {
                D_801A2550_s.delay2 = 6;
                if (!(D_801A2550_s.held & 0x10000000)) {
                    D_801A2550_s.rate2 = 12;
                }
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->raw[0] == 0) {
            w->raw[0] = 3;
        } else {
            w->raw[0]--;
        }
    }
    if (D_801A2550_s.press & 0x40004000) {
        if (D_801A2550_s.press & 0x4000) {
            D_801A2550_s.latch |= 0x4000;
            if (D_801A2574 != 0) {
                D_801A2550_s.delay = 6 / D_801A2574;
                if (!(D_801A2550_s.held & 0x4000)) {
                    D_801A2550_s.rate = 12 / D_801A2574;
                }
            } else {
                D_801A2550_s.delay = 6;
                if (!(D_801A2550_s.held & 0x4000)) {
                    D_801A2550_s.rate = 12;
                }
            }
        }
        if (D_801A2550_s.press & 0x40000000) {
            D_801A2550_s.latch |= 0x40000000;
            if (D_801A2574 != 0) {
                D_801A2550_s.delay2 = 6 / D_801A2574;
                if (!(D_801A2550_s.held & 0x40000000)) {
                    D_801A2550_s.rate2 = 12 / D_801A2574;
                }
            } else {
                D_801A2550_s.delay2 = 6;
                if (!(D_801A2550_s.held & 0x40000000)) {
                    D_801A2550_s.rate2 = 12;
                }
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->raw[0] == 3) {
            w->raw[0] = 0;
        } else {
            w->raw[0]++;
        }
    }
    if (D_801A2550_s.press & 0x400040) {
        D_801A2550 |= 0x400000;
        D_801A2550 |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        switch (w->raw[0]) {
        case 0:
            D_80048630 = D_80048538[0];
            func_800AE838();
            D_800415CC[0] = D_80048538[1].w[0];
            D_800415CC[3] = D_80048538[1].w[3];
            D_800415CC[2] = D_80048538[1].w[2];
            D_800415CC[1] = D_80048538[1].w[1];
            D_800415CC[4] = D_80048538[1].w[10];
            D_800415CC[7] = D_80048538[1].w[8];
            D_800415CC[5] = D_80048538[1].w[9];
            D_800415CC[6] = D_80048538[1].w[11];
            D_800415CC[8] = D_80048538[1].w[4];
            D_800415CC[9] = D_80048538[1].w[6];
            D_800415CC[10] = D_80048538[1].w[5];
            D_800415CC[11] = D_80048538[1].w[7];
            o->update = func_800AA520;
            break;
        case 1:
        case 2:
            if (w->raw[0] == 2) {
                func_800AE184();
                D_80042AA8 = D_80040E60[1];
                D_80031BD4 = D_800419B8[1];
                func_80083A58();
                D_80039D20 = D_80040E38[1];
                D_80048630 = D_80048538[1];
                D_801A2570 = 1;
                func_800AAC20(1, o);
            } else {
                D_80042AA8 = D_80040E60[0];
                D_80031BD4 = D_800419B8[0];
                func_80083A58();
                D_80039D20 = D_80040E38[0];
                D_80048630 = D_80048538[0];
                D_801A2570 = 0;
                func_800AAC20(0, o);
            }
            func_8005BA2C(72, 80, 136, 120, 9, 0, 0);
            SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
            o->update = func_800AB938;
            w->raw[1] = 0;
            break;
        case 3:
            if (D_80048663[0] == 0) {
                w->ring.item[1] = func_8005CAE0(D_800BDD4C, 160, 140, 2);
                w->ring.item[2] = func_8005CAE0(D_800BDD58, 300, 140, 2);
                w->ring.item[1]->light = 11;
                w->ring.item[2]->light = 11;
                o->w.raw[6] = D_80039CAE;
                o->update = func_800AB584;
            } else {
                SsUtKeyOn(D_801A2318, 0, 7, 0x3C, 0, 0x7F, 0x7F);
            }
            break;
        case 4:
            if (D_801A2568 & 0x40) {
                func_800ADC4C(D_8004D3E8, &w->raw[2], 12, 1, 1);
            } else {
                func_800ADC4C(D_8004D3E8, &w->raw[2], 12, 1, 0);
            }
            o->update = func_800AD868;
            break;
        }
    } else if (D_801A2550_s.press & 0x200020) {
        D_801A2550 |= D_801A2550_s.press;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        if (D_801A2550_s.press & 0x20) {
            func_800ADC4C(D_8004D3E8, &w->raw[2], 12, 1, 1);
        } else {
            func_800ADC4C(D_8004D3E8, &w->raw[2], 12, 1, 0);
        }
        o->update = func_800AD868;
    }
}
