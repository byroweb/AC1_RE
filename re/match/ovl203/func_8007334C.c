#include "ac1/ac1.h"

extern AcEntity D_8019B550[2];
extern s32 func_80012794(s32, s32, s32, s32, AcPos *, s32, s32);

/* Actor (not self) that func_80012794 reports; with first == 0 the first one,
 * otherwise the last one. */
AcEntity *func_8007334C(AcEntity *self, s32 a, s32 b, s32 c, s32 first) {
    AcEntity *e;
    AcEntity *found;
    s32 i;

    e = D_8019B550;
    found = 0;
    i = 2;
    do {
        if (self != e && e->def != 0 && !(*(u16 *)((u8 *)e + 0x32) & 4)) {
            if (func_80012794(a, b, c, first, &e->pos, *(u16 *)((u8 *)e + 0x6A), *(u16 *)((u8 *)e + 0x6C)) != 0) {
                if (first == 0) {
                    return e;
                }
                found = e;
            }
        }
        e++;
    } while (--i);
    return found;
}
