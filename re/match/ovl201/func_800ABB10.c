#include "ac1/ac1.h"

typedef struct PadRep {
    u32 latch;                  /* +0x00 0x801A2550 */
    s16 delay;                  /* +0x04 */
    s16 rate;                   /* +0x06 */
    u8 pad08[0x10];
    u32 press;                  /* +0x18 0x801A2568 */
    u32 held;                   /* +0x1C 0x801A256C */
} PadRep;

extern PadRep D_801A2550_s;
extern u32 D_801A2550;
extern u32 D_801A2568;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern s32 D_800485A0;
extern s32 D_800485A4;
extern s32 D_800485F0;
extern s32 D_800485F4;
extern u8 D_80031A58;
extern u8 D_80031A5A;
extern u8 D_80031A5B;
extern u8 D_80031A63;
extern u8 D_8004D4A4[];
extern u8 D_8004D4C4[];
extern u8 D_8004D4E8[];
extern void func_80083A58(void);
extern u8 func_80084290(void);
extern void func_80050AF4(s32, s32);
extern void func_80051450(s32, s32);
extern void func_800506EC(s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_8005DED8(UiObj *, void (*)(UiObj *), void *, s32);
extern void func_800AC4A8();
extern void func_800AC0B8(UiObj *);
extern void func_800AC58C(UiObj *);
extern void func_800AC728(UiObj *);
extern void func_800AC8F4(UiObj *);
extern void func_800ACC44(UiObj *);
extern void func_800AD40C(UiObj *);

void func_800ABB10(UiObj *o)
{
    RECT r;
    u8 *w = o->w.raw;

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
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w[1] == 0) {
            w[1] = 4;
        } else {
            w[1]--;
        }
    }
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
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w[1] == 4) {
            w[1] = 0;
        } else {
            w[1]++;
        }
    }
    if (D_801A2568 & 0x40) {
        D_801A2550 |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        switch (w[1]) {
        case 0:
            func_80083A58();
            D_800485F0 = 0;
            D_800485F4 = 0;
            if (D_800485A0 == 0 && D_800485A4 == 0) {
                func_8005DED8(o, func_800ABB10, D_8004D4A4, 12);
                o->update = ui_obj_nop2;
            } else {
                func_800AC4A8();
                o->update = func_800AD40C;
            }
            break;
        case 1:
            func_80083A58();
            if (D_800485F0 == 0) {
                func_8005DED8(o, func_800ABB10, D_8004D4C4, 12);
                o->update = ui_obj_nop2;
            } else if (D_800485A0 == 0 && D_800485A4 == 0) {
                func_8005DED8(o, func_800ABB10, D_8004D4A4, 12);
                o->update = ui_obj_nop2;
            } else {
                r.x = 384;
                r.y = 256;
                r.w = 64;
                r.h = 64;
                MoveImage(&r, 512, 256);
                r.x = 320;
                r.y = 482;
                r.w = 256;
                r.h = 1;
                MoveImage(&r, 320, 511);
                DrawSync(0);
                func_800AC4A8();
                func_80050AF4(13, func_80084290() + 132);
                func_80051450(17, func_80084290());
                o->update = func_800AC8F4;
            }
            break;
        case 2:
            if (D_80031A58 == 0xFF || D_80031A5A == 0xFF || D_80031A5B == 0xFF || D_80031A63 == 0xFF) {
                func_8005DED8(o, func_800ABB10, D_8004D4E8, 12);
                o->update = ui_obj_nop2;
            } else {
                func_800AC4A8();
                o->update = func_800AC728;
            }
            break;
        case 3:
            if (D_80031A58 == 0xFF || D_80031A5A == 0xFF || D_80031A5B == 0xFF || D_80031A63 == 0xFF) {
                func_8005DED8(o, func_800ABB10, D_8004D4E8, 12);
                o->update = ui_obj_nop2;
            } else {
                func_800AC4A8();
                o->update = func_800AC58C;
            }
            break;
        case 4:
            o->w.raw[7] = 0;
            o->update = func_800ACC44;
            break;
        }
    } else if (D_801A2568 & 0x20) {
        D_801A2550 |= 0x200020;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0x48, 0x50, 0x88, 0x78, 9);
        o->update = func_800AC0B8;
    }
}
