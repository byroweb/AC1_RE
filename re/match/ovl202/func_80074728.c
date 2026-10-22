#include "ac1/ac1.h"

extern s32 func_80012E5C(SVECTOR *a, s32 b, s32 c, AcPos *pos, s32 r, s32 h);

AcEntity *func_80074728(AcEntity *self, SVECTOR *a, s32 b, s32 c)
{
    AcEntity *e;
    s32 i;

    e = g_ac_entities;
    for (i = 16; i != 0; e++, i--) {
        if (e->def != 0 && e != self && !(*(u16 *)((u8 *)e + 0x32) & 4)) {
            if (func_80012E5C(a, b, c, &e->pos, *(u16 *)((u8 *)e + 0x6A), *(u16 *)((u8 *)e + 0x6C)) != 0) {
                goto found;
            }
        }
    }
    return 0;
found:
    return e;
}
