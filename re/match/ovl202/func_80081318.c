#include "ac1/ac1.h"

typedef struct {
    u8 pad[0xCC];
    SVECTOR v;          /* +0xCC */
} AcDash;

extern void func_80081088(AcEntity *e, s32 a, s32 b, s32 c, SVECTOR *v);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);

s32 func_80081318(AcEntity *e)
{
    switch (((u8 *)&e->mover)[0]) {
    case 0:
    case 2:
    case 4:
    case 6:
    case 9:
    case 12:
    case 15:
    case 17:
    case 19:
        if (((AcDash *)e)->v.vx != 0 || ((AcDash *)e)->v.vz != 0) {
            func_80081088(e, 0, 0, 256, &((AcDash *)e)->v);
            return 0;
        }
        break;
    case 21:
        func_8004E98C(&e->mover, 0, 1, 512);
        ((u8 *)&e->mover)[2] = 4;
        ((AcDash *)e)->v.vy = 0;
        ((AcDash *)e)->v.vx = 0;
        return 1;
    }
    return 1;
}
