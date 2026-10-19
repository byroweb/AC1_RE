/* ovl201 func_8007CECC - update step: once the object built by func_8005C2D0
 * is gone (the lookup returns the g_ui_none sentinel), start a sound and move
 * on to func_8007CF64. */
#include "ac1/ac1.h"
extern s16 D_801A2318;
extern void func_8005C2D0(void);
extern void func_8005BA2C(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_8007CF64(UiObj *o);
void func_8007CECC(UiObj *o)
{
    if (obj_find_by_handle((void *)func_8005C2D0) == &g_ui_none) {
        func_8005BA2C(0x20, 0x28, 0x88, 0x80, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        o->update = func_8007CF64;
    }
}
