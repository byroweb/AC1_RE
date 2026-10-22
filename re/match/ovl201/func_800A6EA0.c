#include "ac1/ac1.h"

extern void func_80051450(s32, s32);
extern void func_800506EC(s32);
extern void func_80051078(void *);
extern void func_8005B2CC(void);
extern void func_80050E94(void);
extern void func_80050C18(void);
extern void func_800A2D0C(s32);
extern void func_8005CC48();
extern void func_80072838();
extern s32 D_801A2574;
extern void *D_801F36D0[];
extern s32 D_801BC478[];
extern u8 D_80039CAC;
extern u8 D_80039CAD;

void func_800A6EA0(UiObj *o)
{
    s16 i;

    func_80051450(4, 5);
    obj_find_by_handle(&func_8005CC48)->kill = 1;
    obj_find_by_handle(&func_80072838)->kill = 1;
    func_800506EC(4);
    obj_find_by_id(3)->kill = 1;
    func_800506EC(6);
    obj_find_by_id(5)->kill = 1;
    func_800506EC(8);
    obj_find_by_id(7)->kill = 1;
    func_800506EC(12);
    obj_find_by_id(11)->kill = 1;
    func_800506EC(10);
    obj_find_by_id(9)->kill = 1;
    func_800506EC(2);
    obj_find_by_id(1)->kill = 1;
    D_801A2574 = 0;
    o->kill = 1;
    for (i = 3; i < 15; i++) {
        if (D_801F36D0[i] != 0) {
            func_80051078(D_801F36D0[i]);
            D_801F36D0[i] = 0;
            D_801BC478[i] = 0;
        }
    }
    D_80039CAC = 0;
    D_80039CAD = 0;
    func_8005B2CC();
    func_80050E94();
    func_80050C18();
    func_800A2D0C(1);
}
