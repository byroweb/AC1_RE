#include "ac1/ac1.h"
/* ovl201 0x8007CFC0 - build the invisible menu object of the next screen
 * (update func_8007D254), print the D_800B827C line of the current variant
 * and keep the caller's two arguments and the D_80039CA3 cursor. */

typedef struct {
    void *owner;                /* +0x7C */
    s32 arg;                    /* +0x80 */
    u8 cursor;                  /* +0x84 */
} MenuWork;

extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern char *D_800B827C[];
extern void func_8005D15C(char *s, s32 x, s32 y);
extern void func_8007D07C(UiObj *o);
extern void func_8007D254(UiObj *o);

UiObj *func_8007CFC0(void *owner, s32 arg)
{
    UiObj *o;

    o = ui_obj_alloc(0);
    o->ctor = func_8007CFC0;
    o->update = func_8007D254;
    o->visible = 0;
    o->attached = 0;
    o->has_xform = 0;
    o->destroy = ui_obj_nop;
    o->parent = 0;
    func_8005D15C(D_800B827C[D_80039CA2], 960, 128);
    func_8007D07C(o);
    ((MenuWork *)&o->w)->owner = owner;
    ((MenuWork *)&o->w)->arg = arg;
    ((MenuWork *)&o->w)->cursor = D_80039CA3;
    return o;
}
