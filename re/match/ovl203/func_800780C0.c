#include "ac1_types.h"
/* ovl203 func_800780C0 - for each of the two 0x39C-byte player blocks at
 * D_80040FA0: make it current (D_80039D18), refresh it, then call the attached
 * object's handler at +0x54 and, if its first word is set, func_80077798 with
 * the player index. */

typedef struct Func800780C0Obj {
    s32 active;                                         /* +0x00 */
    u8 pad[0x50];
    void (*handler)(struct Func800780C0Obj *, s32);     /* +0x54 */
} Func800780C0Obj;

typedef struct {
    u8 pad[0x394];
    Func800780C0Obj *obj;   /* +0x394 */
    u8 pad2[0x39C - 0x398];
} Func800780C0Player;

extern Func800780C0Player D_80040FA0[];
extern Func800780C0Player *D_80039D18;
extern u8 D_800895C8;

extern void func_80077E10(void);
extern void func_8007738C(void);
extern void func_80077C80(void);
extern void func_80077798(Func800780C0Obj *obj, s32 n);

void func_800780C0(void)
{
    Func800780C0Obj *obj;

    func_80077E10();
    if (D_800895C8 == 0) {
        func_8007738C();
    }

    D_80039D18 = &D_80040FA0[0];
    func_80077C80();
    obj = D_80039D18->obj;
    if (obj->handler != 0) {
        obj->handler(obj, 0);
    }
    if (obj->active != 0) {
        func_80077798(obj, 0);
    }

    D_80039D18 = &D_80040FA0[1];
    func_80077C80();
    obj = D_80039D18->obj;
    if (obj->handler != 0) {
        obj->handler(obj, 0);
    }
    if (obj->active != 0) {
        func_80077798(obj, 1);
    }
}
