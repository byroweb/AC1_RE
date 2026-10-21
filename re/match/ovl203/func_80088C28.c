#include "ac1/ac1.h"

extern AcEntity D_8019B550[2];  /* overlay 203 actor pair (cf. ovl202 g_ac_entities) */

void func_80088C28(AcEntity *e, s32 msg, ...) {
    void (*fn)(AcEntity *, s32, s32);

    if (e >= D_8019B550 && e <= &D_8019B550[1]) {
        fn = *(void (**)(AcEntity *, s32, s32))((u8 *)e + 0x54);
        if (fn != 0) {
            fn(e, msg, (&msg)[1]);
        }
    }
}
