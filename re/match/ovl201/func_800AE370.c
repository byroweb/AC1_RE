#include "ac1/ac1.h"
#include "ac1/uidialog.h"

extern UiDialog D_801A2308;
extern void func_8005BA2C(s32, s32, s32, s32, s32, s32, s32);
extern short SsUtKeyOn(short vabId, short prog, short tone, short note, short fine, short voll, short volr);
extern void func_800AE418(UiObj *);

void func_800AE370(UiObj *o) {
    if (obj_find_by_id(D_801A2308.obj_id) == &g_ui_none) {
        func_8005BA2C(32, 100, 256, 48, D_801A2308.obj_id, 1, 0);
        SsUtKeyOn(D_801A2308.vab, 0, 7, 60, 0, 127, 127);
        o->update = func_800AE418;
    }
}
