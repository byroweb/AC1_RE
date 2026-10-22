#include "ac1/ac1.h"

extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);
extern void func_800506EC(s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_800A74A4(UiObj *);
extern void func_800A7818(UiObj *);

void func_800A737C(UiObj *o)
{
    o->w.raw[0xD] = 0;
    func_800309A8(2, 0, 0);
    func_800309A8(1, 1, 2);
    while (func_800309A8(3, 1, 0) == 0) {
        o->w.raw[0xD] = 1;
        if (func_800309A8(0, 0, 0) & 0x80) {
            o->update = func_800A7818;
            func_800309A8(2, 0, 0);
            VSync(0);
            func_800309A8(1, 1, 0);
            VSync(0);
            func_800506EC(12);
            obj_find_by_id(11)->kill = 1;
            func_8005C2D0(28, 170, 264, 32, 11);
            return;
        }
    }
    VSync(0);
    func_800309A8(1, 1, 0);
    VSync(0);
    o->update = func_800A74A4;
}
