#include "ac1/ac1.h"

extern u16 D_80041230;
extern u16 D_80041232;
extern u16 D_80041234;
extern u16 D_80041236;
extern u16 D_80041238;
extern u16 D_8004123A;
extern u16 D_8004123C;
extern u16 D_8004123E;
extern u16 D_80041240;
extern u16 D_80041242;
extern u16 D_80041244;
extern u16 D_80041246;
extern u16 D_800415CC;
extern u16 D_800415CE;
extern u16 D_800415D0;
extern u16 D_800415D2;
extern u16 D_800415D4;
extern u16 D_800415D6;
extern u16 D_800415D8;
extern u16 D_800415DA;
extern u16 D_800415DC;
extern u16 D_800415DE;
extern u16 D_800415E0;
extern u16 D_800415E2;

/* Swap the active pad mapping (0x80041230) with the alternate one (0x800415CC). */
void func_800AE184(void) {
    u16 t;

    t = D_80041230;
    D_80041230 = D_800415CC;
    D_800415CC = t;
    t = D_80041232;
    D_80041232 = D_800415CE;
    D_800415CE = t;
    t = D_80041234;
    D_80041234 = D_800415D0;
    D_800415D0 = t;
    t = D_80041236;
    D_80041236 = D_800415D2;
    D_800415D2 = t;
    t = D_80041238;
    D_80041238 = D_800415D4;
    D_800415D4 = t;
    t = D_8004123A;
    D_8004123A = D_800415D6;
    D_800415D6 = t;
    t = D_8004123C;
    D_8004123C = D_800415D8;
    D_800415D8 = t;
    t = D_8004123E;
    D_8004123E = D_800415DA;
    D_800415DA = t;
    t = D_80041240;
    D_80041240 = D_800415DC;
    D_800415DC = t;
    t = D_80041242;
    D_80041242 = D_800415DE;
    D_800415DE = t;
    t = D_80041244;
    D_80041244 = D_800415E0;
    D_800415E0 = t;
    t = D_80041246;
    D_80041246 = D_800415E2;
    D_800415E2 = t;
}
