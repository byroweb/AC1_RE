/* ovl201 func_800AD868 - two-step sequencer.  Step 0 retires the object built
 * by func_8005CC48 and the objects with ids 1, 3, 5 and 7 (a missing one is
 * the g_ui_none sentinel, so the stores are safe), retires itself and resets
 * the screen; step 1 hands over to func_800A9AFC. */
#include "ac1/ac1.h"
extern u8 D_80039CAB;
extern s32 D_801A2574;
extern void func_8005CC48(void);
extern void func_80051450(s32 a, s32 b);
extern void func_800506EC(s32 kind);
extern void func_8005B2CC(void);
extern void func_800AD96C(UiObj *o);
extern void func_80050E94(void);
extern void func_80050C18(void);
extern void func_800A2D0C(s32 a);
extern void func_800A9AFC(UiObj *o);
void func_800AD868(UiObj *o)
{
    if (o->w.seq.state == 0) {
        func_80051450(4, 5);
        obj_find_by_handle((void *)func_8005CC48)->kill = 1;
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_800506EC(2);
        obj_find_by_id(1)->kill = 1;
        D_801A2574 = 0;
        o->kill = 1;
        D_80039CAB = 0;
        func_8005B2CC();
        func_800AD96C(o);
        func_80050E94();
        func_80050C18();
        func_800A2D0C(1);
    } else if (o->w.seq.state == 1) {
        o->update = func_800A9AFC;
    }
}
