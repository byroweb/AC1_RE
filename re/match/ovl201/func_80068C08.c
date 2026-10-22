#include "ac1/ac1.h"

typedef struct { u8 c[4]; } EmbRgb;
extern EmbRgb D_8017E5D8[];
extern EmbRgb D_8017E618;
extern RECT D_800B7FDC;
extern RECT D_800B7FE4;
extern s16 D_801A2318;
extern s32 D_801A2550;
extern s32 D_801A2568;
extern s32 D_801A256C;
short SsUtKeyOn(short, short, short, short, short, short, short);
void box_init(s32, s32, s32, s32, s32, s32, s32);
void func_8005BA2C(s32, s32, s32, s32, s32, s32, s32);
void func_8006BA08(UiObj *);
void func_8006CC78(UiObj *);
void func_8006D1D8(void);
void func_8006F5E8(void);
void func_8006F670(UiObj *);
void func_8006F79C(UiObj *, s32);
void func_80068E14(UiObj *);
void func_8006B250(UiObj *);

void func_80068C08(UiObj *o)
{
    func_8006D1D8();
    if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        o->w.raw[0x90 - 0x7C] = 0;
        box_init(0x9B, 0x17, 0x97, 0xBC, 5, 0, 0);
        box_init(0x1B, 0x27, 0x50, 0x6C, 7, 0, 0);
        func_8005BA2C(0x50, 0x32, 0x68, 0x90, 9, 0, 0);
        o->update = func_80068E14;
    } else if ((D_801A2568 & 0x80) && !(D_801A256C & 0x80)) {
        D_8017E618 = D_8017E5D8[o->w.raw[0x86 - 0x7C]];
        func_8006F5E8();
        func_8006F670(o);
        o->w.raw[0x96 - 0x7C] = 0;
        func_8006F79C(o, 0);
        func_8006F79C(o, 1);
        func_8006F79C(o, 2);
        func_8006CC78(o);
        o->update = func_8006B250;
    }
    if ((D_801A2568 & 0x10C) == 0x10C) {
        D_801A2550 |= D_801A2568;
        MoveImage(&D_800B7FDC, 0x1C0, 0xC0);
        MoveImage(&D_800B7FE4, 0x170, 0xE1);
    }
    func_8006BA08(o);
}
