#include "ac1/ac1.h"
/* ovl201 0x8004FDF4 - build the invisible root object of this screen (update
 * func_8004FF14), upload its CLUT and texture strip and open a full-screen
 * box. */

extern u8 D_80041B14;
extern u8 D_80039D21;
extern u32 D_8004AC18[];
extern u32 D_80042B70[];
extern void func_800866A8(void);
extern void func_8004FF14(UiObj *o);
extern int LoadImage(RECT *rect, u32 *p);
extern int DrawSync(int mode);
extern void box_init(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);

UiObj *func_8004FDF4(void)
{
    UiObj *o;

    o = ui_obj_alloc(0);
    o->ctor = func_8004FDF4;
    o->update = func_8004FF14;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    o->w.raw[1] = 0;
    D_80041B14 = 0;
    func_800866A8();
    {
        RECT tex = { 448, 192, 16, 64 };
        RECT clut = { 368, 225, 16, 1 };

        D_80039D21 = 1;
        LoadImage(&clut, D_8004AC18);
        LoadImage(&tex, D_80042B70);
    }
    DrawSync(0);
    box_init(0, 0, 320, 240, 1, 0, 0);
    return o;
}
