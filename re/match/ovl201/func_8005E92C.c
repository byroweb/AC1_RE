#include "ac1/ac1.h"

extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern RECT D_800B7450;
extern RECT D_800B7458;
extern RECT D_800B7460;
extern RECT D_800B7468;
extern u32 D_8004AC18[];
extern u32 D_80042B70[];
extern int MoveImage(RECT *rect, int x, int y);
extern int StoreImage(RECT *rect, u32 *p);
extern int DrawSync(int mode);
extern void func_800506EC(s16);
extern void func_8005EAA0(UiObj *);

void func_8005E92C(void) {
    UiObj *o;
    s16 i;

    for (i = 2; i < 21; i += 2) {
        func_800506EC(i);
    }
    for (i = 1; i < 20; i += 2) {
        obj_find_by_id(i)->kill = 1;
    }
    o = ui_obj_alloc(0);
    o->ctor = func_8005E92C;
    o->update = func_8005EAA0;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    if ((D_80031A5E != 0xFF && D_80031A5E >= 31) || (D_80031A5F != 0xFF && D_80031A5F >= 31)) {
        MoveImage(&D_800B7450, 0x300, 0x1C0);
        MoveImage(&D_800B7458, 0, 0x1ED);
        MoveImage(&D_800B7458, 0, 0x1EE);
        DrawSync(0);
    }
    StoreImage(&D_800B7468, D_8004AC18);
    StoreImage(&D_800B7460, D_80042B70);
    DrawSync(0);
}
