#include "ac1/ac1.h"

extern void func_800506EC(s32 id);
extern void func_8005616C(UiObj *o);
extern void func_80056770(UiObj *o);
extern void func_80056C48(UiObj *o);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern s32 func_800685A8(u8 a);
extern void func_8005BA2C(void);
extern void func_8005D8D4(void);

void func_80056B60(UiObj *o)
{
    if (func_800685A8(o->w.raw[0x28]) != 0) {
        obj_find_by_handle(func_8005D8D4)->kill = 1;
        obj_find_by_handle(func_8005BA2C)->kill = 1;
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        o->update = func_80056770;
        return;
    }
    if (o->w.raw[0x27] == 0) {
        func_800506EC(4);
        func_800506EC(6);
        obj_find_by_id(3)->kill = 1;
        obj_find_by_id(5)->kill = 1;
        o->update = func_80056C48;
    } else if (o->w.raw[0x27] == 1) {
        o->update = func_8005616C;
    }
}
