#include "ac1/ac1.h"
/* ovl201 0x800A3564 - two-item menu confirm: the cursor row (y 0xB0 / 0xC0)
 * picks between restarting (reset the D_80039D20..26 flags, reopen the box,
 * go to func_800A38CC) and leaving (func_800A88F8 or func_800A4F44). */
extern s8 D_80039C5E;
extern u8 D_80039D2A;
extern s8 D_80039D20;
extern s8 D_80039D21;
extern s8 D_80039D22;
extern s8 D_80039D23;
extern s8 D_80039D24;
extern s8 D_80039D25;
extern s8 D_80039D26;
extern s16 D_801A2318;
extern s32 D_801A2568;
extern s32 func_800309A8(s32 a, s32 b, s32 c);
extern void func_800506EC(s32 kind);
extern void func_80050804(s32 kind, s32 id);
extern void func_8005ACF4(void);
extern void func_8005AE8C(void);
extern void func_8005BA2C(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);
extern void func_800689E0(void);
extern void func_800A38CC(UiObj *o);
extern void func_800A4F44(void);
extern void func_800A88F8(void);
extern void func_800AE7A4(void);
extern void func_800AE838(void);
extern void func_800B1F20(void);

typedef struct {
    u8 pad7c[0x14];
    UiObj *cursor;      /* +0x90 */
    u8 pad94[0xA];
    u8 unk9E;           /* +0x9E */
} MenuWork;

void func_800A3564(UiObj *o)
{
    s32 alt;
    MenuWork *w = (MenuWork *)&o->w;

    alt = func_800309A8(0, 0, 0) & 0x80;
    if (D_801A2568 & 2) {
        alt = 1;
    }
    if (w->cursor->pos.vy == 0xB0) {
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        D_80039D2A = 0xFF;
        D_80039C5E = 0;
        func_8005ACF4();
        func_800B1F20();
        func_800AE7A4();
        func_800AE838();
        D_80039D20 = 0;
        D_80039D21 = 0;
        D_80039D22 = 0;
        D_80039D23 = 0;
        D_80039D24 = 0;
        D_80039D25 = 0;
        D_80039D26 = 0;
        func_800506EC(2);
        func_80050804(4, 0);
        func_80050804(4, 1);
        func_80050804(4, 4);
        func_8005BA2C(0x5C, 0x60, 0x88, 0x30, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        w->unk9E = 0;
        o->update = func_800A38CC;
    } else if (w->cursor->pos.vy == 0xC0) {
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        if (alt != 0) {
            func_800689E0();
            func_8005AE8C();
            D_80039D20 = 0;
            D_80039D21 = 0;
            D_80039D22 = 0;
            D_80039D23 = 0;
            D_80039D24 = 0;
            D_80039D25 = 0;
            D_80039D26 = 0;
            func_800506EC(2);
            o->kill = 1;
            func_80050804(4, 0);
            D_80039C5E = 2;
            func_800A88F8();
        } else {
            func_800689E0();
            func_8005AE8C();
            D_80039D20 = 0;
            D_80039D21 = 0;
            D_80039D22 = 0;
            D_80039D23 = 0;
            D_80039D24 = 0;
            D_80039D25 = 0;
            D_80039D26 = 0;
            func_800506EC(2);
            o->kill = 1;
            func_80050804(4, 0);
            D_80039C5E = 1;
            func_800A4F44();
        }
    }
}
