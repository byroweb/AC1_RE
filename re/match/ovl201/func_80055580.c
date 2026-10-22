#include "ac1/ac1.h"

extern void func_800506EC(s32 id);
extern void func_800542F0(UiObj *o);
extern void func_80054A68(UiObj *o);
extern void func_800556A8(UiObj *o);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern s32 func_800685A8(u8 a);
extern u8 D_801A257C;
extern void func_8005BA2C(void);
extern void func_8005D8D4(void);

void func_80055580(UiObj *o)
{
    u8 n;

    if (o->w.raw[0x27] == 0) {
        func_800506EC(4);
        func_800506EC(6);
        func_800506EC(8);
        obj_find_by_id(3)->kill = 1;
        obj_find_by_id(5)->kill = 1;
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(0x10, 0x28, 0x60, 0x18, 3);
        func_8005C2D0(0x10, 0x40, 0x120, 0x18, 5);
        n = D_801A257C;
        if (o->w.raw[0x2B] != 0) {
            n++;
        }
        if (n > 5) {
            n = 5;
        }
        func_8005C2D0(0x10, 0x58, 0x120, n * 0x18, 7);
        o->update = func_800556A8;
    } else if (o->w.raw[0x27] == 1) {
        o->update = func_800542F0;
    }
}
