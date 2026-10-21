#include "ac1/ac1.h"

extern s32 func_80012794(SVECTOR *a, s32 b, s32 c, SVECTOR *d, AcPos *pos, s32 r, s32 h);

AcEntity *func_80074620(AcEntity *self, SVECTOR *a, s32 b, s32 c, SVECTOR *d)
{
    AcEntity *e;
    AcEntity *hit;
    s32 i;

    e = g_ac_entities;
    hit = 0;
    for (i = 16; i != 0; e++, i--) {
        if (self == e || e->def == 0 || (*(u16 *)((u8 *)e + 0x32) & 4)) {
            continue;
        }
        if (func_80012794(a, b, c, d, &e->pos, *(u16 *)((u8 *)e + 0x6A), *(u16 *)((u8 *)e + 0x6C)) != 0) {
            if (d == 0) {
                return e;
            }
            hit = e;
        }
    }
    return hit;
}
