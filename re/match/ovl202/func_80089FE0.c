#include "ac1/ac1.h"

void func_80089FE0(AcEntity *e, s32 msg, ...) {
    void (*fn)(AcEntity *, s32, s32, s32);

    if (e >= g_ac_entities && e <= &g_ac_entities[15]) {
        fn = *(void (**)(AcEntity *, s32, s32, s32))((u8 *)e + 0x54);
        if (fn != 0) {
            fn(e, msg, (&msg)[1], (&msg)[2]);
        }
    }
}
