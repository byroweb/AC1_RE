#include "ac1/ac1.h"

typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Func801C8CBCBox;

void func_801C8CBC(u8 *o, Func801C8CBCBox *box, s32 n)
{
    AcEntity *e;
    s16 x, y, z;

    e = *(AcEntity **)(o + 0x394);
    x = e->pos.x;
    if (x >= box->lo.vx && box->hi.vx >= x) {
        y = e->pos.y;
        if (y >= box->lo.vy && box->hi.vy >= y) {
            z = e->pos.z;
            if (z >= box->lo.vz && box->hi.vz >= z) {
                *(u16 *)(o + 0x10) |= 1 << n;
            }
        }
    }
    switch (n) {
    case 1:
        *(Func801C8CBCBox *)(o + 0x226) = *box;
        break;
    case 2:
        *(Func801C8CBCBox *)(o + 0x236) = *box;
        break;
    }
}
