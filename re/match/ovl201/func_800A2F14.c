#include "ac1/ac1.h"
typedef struct { s16 x, y, z; } P3;
extern P3 D_800B8F24[3];
extern void func_800A2FD0(UiObj *o);

void func_800A2F14(UiObj *o)
{
    D_800B8F24[0].x -= 8;
    D_800B8F24[0].y -= 8;
    D_800B8F24[1].x -= 8;
    D_800B8F24[1].y -= 8;
    D_800B8F24[2].x -= 8;
    D_800B8F24[2].y -= 8;
    if (D_800B8F24[0].x < 0x40) {
        ((UiObj *)o->w.list.src)->kill = 1;
        (*(UiObj **)&o->w.raw[4])->kill = 1;
        o->update = func_800A2FD0;
    }
}
