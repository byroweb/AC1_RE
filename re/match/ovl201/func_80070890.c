#include "ac1/ac1.h"
/* ovl201 0x80070890 - build the invisible root object of the mission-launch
 * screen (update func_800709D4): func_8007092C, func_8005CC48, a title
 * image (func_8005CAE0) and the kind byte at +0x88. */

typedef struct {
    u8 pad7c[4];
    UiObj *title;               /* +0x80 */
    u8 pad84[4];
    u8 kind;                    /* +0x88 */
} LaunchWork;

extern u8 D_800B83B8[];
extern void func_800709D4(UiObj *o);
extern void func_8007092C(void);
extern void func_8005CC48(void);
extern UiObj *func_8005CAE0(void *e, s32 x, s16 y, s32 kind);

UiObj *func_80070890(u8 kind)
{
    UiObj *o;

    o = ui_obj_alloc(0);
    o->ctor = func_80070890;
    o->update = func_800709D4;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    func_8007092C();
    func_8005CC48();
    ((LaunchWork *)&o->w)->title = func_8005CAE0(D_800B83B8, 32, 16, 2);
    ((LaunchWork *)&o->w)->kind = kind;
    return o;
}
