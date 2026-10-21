#include "ac1/ac1.h"

void func_80089F7C(AcEntity *e, s32 a, ...)
{
    if (e >= g_ac_entities && e <= &g_ac_entities[15]) {
        if (e->fn_move) {
            e->fn_move(e, a, *(s32 *)(&a + 1));
        }
    }
}
