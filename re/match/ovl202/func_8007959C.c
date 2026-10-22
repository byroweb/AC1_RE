#include "ac1/ac1.h"

extern void func_8007922C(void);
extern void func_80079524(void);
extern void func_8007863C(void);
extern void func_8008D69C(void);
extern void func_80079370(void);
extern s32 D_80040FA8;
extern s32 D_80040FAC;
extern s32 D_801A5DC4;

typedef void (*MoveFn)(AcEntity *, s32);

void func_8007959C(void)
{
    AcEntity *e;
    s32 i;

    func_8007922C();
    func_80079524();
    func_8007863C();
    if ((D_80040FA8 & 0x100) && !(D_80040FAC & 0x100) && !(g_mission_flags & 0x1400)) {
        func_8008D69C();
    }
    func_80079370();
    e = g_ac_entities;
    if (e->fn_move != 0) {
        ((MoveFn)e->fn_move)(e, 0);
    }
    e = &g_ac_entities[D_801A5DC4 % 16];
    if (e != g_ac_entities && e->def != 0) {
        if (e->fn_move != 0) {
            ((MoveFn)e->fn_move)(e, 0);
        }
    }
    e = g_ac_entities;
    i = 0;
    do {
        if (e->def != 0) {
            ac_entity_update(e, (void *)i);
        }
        i++;
        e++;
    } while (i < 16);
}
