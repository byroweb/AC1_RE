#include "ac1/ac1.h"

extern AcEntity D_8019B550[2];
extern s32 func_80012E5C(s32, s32, s32, AcPos *, s32, s32);

/* First live actor other than self that func_80012E5C reports a hit on, or 0. */
AcEntity *func_80073454(AcEntity *self, s32 a, s32 b, s32 c) {
    AcEntity *e;
    s32 i;

    e = D_8019B550;
    i = 2;
    do {
        if (e->def != 0 && e != self && !(*(u16 *)((u8 *)e + 0x32) & 4)) {
            if (func_80012E5C(a, b, c, &e->pos, *(u16 *)((u8 *)e + 0x6A), *(u16 *)((u8 *)e + 0x6C)) != 0) {
                return e;
            }
        }
        e++;
    } while (--i);
    return 0;
}
