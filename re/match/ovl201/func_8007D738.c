#include "ac1/ac1.h"
/* ovl201 0x8007D738 - build the invisible menu object of the paint screen
 * (update func_8007D934), print its six entries and keep the caller's two
 * arguments and a cleared cursor. */

typedef struct {
    void *owner;                /* +0x7C */
    s32 arg;                    /* +0x80 */
    u8 cursor;                  /* +0x84 */
} MenuWork;

extern void func_8005D15C(char *s, s32 x, s32 y);
extern void func_8007D840(UiObj *o);
extern void func_8007D934(UiObj *o);

UiObj *func_8007D738(void *owner, s32 arg)
{
    UiObj *o;

    o = ui_obj_alloc(0);
    o->ctor = func_8007D738;
    o->update = func_8007D934;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    func_8005D15C("GENERAL", 1000, 52);
    func_8005D15C("HEAD   ", 1000, 76);
    func_8005D15C("CORE   ", 1000, 100);
    func_8005D15C("ARMS   ", 1000, 124);
    func_8005D15C("LEGS   ", 1000, 148);
    func_8005D15C("DEFAULT", 1000, 172);
    func_8007D840(o);
    ((MenuWork *)&o->w)->owner = owner;
    ((MenuWork *)&o->w)->arg = arg;
    ((MenuWork *)&o->w)->cursor = 0;
    return o;
}
