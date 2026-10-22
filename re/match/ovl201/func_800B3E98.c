#include "ac1/ac1.h"

extern s16 D_801A2318;
extern s32 D_801A2550;
extern s32 D_801A2568;
extern void func_800506EC(s32);
extern void func_80051078(s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_800B3FFC(UiObj *);

void func_800B3E98(UiObj *o)
{
    s32 *trig = &D_801A2568;

    if (*trig & 0x60) {
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550 |= *trig;
        func_800506EC(4);
        func_800506EC(6);
        func_800506EC(8);
        func_800506EC(0xA);
        obj_find_by_id(3)->kill = 1;
        obj_find_by_id(5)->kill = 1;
        obj_find_by_id(7)->kill = 1;
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0x10, 0x40, 0xB0, 0x20, 3);
        func_8005C2D0(0x10, 0x60, 0xB0, 0x30, 5);
        func_8005C2D0(0xC0, 0x10, 0x78, 0x80, 7);
        func_8005C2D0(0x10, 0x90, 0x128, 0x50, 9);
        func_80051078(*(s32 *)&o->w.raw[0x20]);
        o->update = func_800B3FFC;
    }
}
