#include "ac1/ac1.h"

extern void *func_80074800(AcEntity *e, SVECTOR *v, s32 a, s32 b);
extern void *func_80074620(AcEntity *e, SVECTOR *v, s32 r, s32 h, SVECTOR *d);
extern void *func_800733D8(void *self, SVECTOR *v, s32 r, s32 h, SVECTOR *d);

void *func_80074FA4(AcEntity *e, SVECTOR *v)
{
    void *hit;

    hit = func_80074800(e, v, 0, -1);
    if (hit != 0) {
        return hit;
    }
    hit = func_80074620(e, v, *(u16 *)((u8 *)e + 0x6A), *(u16 *)((u8 *)e + 0x6C), 0);
    if (hit != 0) {
        return hit;
    }
    hit = func_800733D8(0, v, *(u16 *)((u8 *)e + 0x6A), *(u16 *)((u8 *)e + 0x6C), 0);
    if (hit != 0) {
        return hit;
    }
    return 0;
}
