#include "ac1/ac1.h"

void func_80073958(void) {
    s32 i;
    AcEntity *e = &g_ac_entities[1];

    i = 15;
    do {
        e->fn_update = 0;
        e->fn_state = 0;
        e->fn_move = 0;
        e->fn_think = 0;
        e->fn_hit = 0;
        e++;
    } while (--i != 0);
}
