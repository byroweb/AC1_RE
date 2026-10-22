#include "ac1/ac1.h"
#include "ac1/uidialog.h"

extern UiDialog D_801A2308;
extern UiPad D_801A2550;
extern short SsUtKeyOn(short vabId, short prog, short tone, short note, short fine, short voll, short volr);
extern void func_800506EC(s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_800AE75C(UiObj *);

void func_800AE644(UiObj *o) {
    if ((D_801A2550.trig & 0x20 && !(D_801A2550.held & 0x20))
        || (D_801A2550.trig & 0x40 && !(D_801A2550.held & 0x40))) {
        SsUtKeyOn(D_801A2308.vab, 0, 1, 60, 0, 127, 127);
        func_800506EC(D_801A2308.sel);
        obj_find_by_id(D_801A2308.obj_id)->kill = 1;
        func_8005C2D0(32, 100, 256, 48, D_801A2308.obj_id);
        o->update = func_800AE75C;
        D_801A2308.active = 0;
        D_801A2308.done();
        D_801A2550.trig &= ~0x60;
    }
}
