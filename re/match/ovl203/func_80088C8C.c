#include "ac1/ac1.h"

/* ovl203: forward a message to an AC (the two-slot entity array of this mode). */
extern AcEntity D_8019B550[];

void func_80088C8C(AcEntity *e, s32 msg, ...)
{
    void (*fn)(AcEntity *, s32, s32, s32);

    if (e >= &D_8019B550[0] && e <= &D_8019B550[1]) {
        fn = (void (*)(AcEntity *, s32, s32, s32))e->fn_move;
        if (fn != 0) {
            fn(e, msg, (&msg)[1], (&msg)[2]);
        }
    }
}
