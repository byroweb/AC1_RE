#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x72];
    s16 h72;            /* +0x72 */
    u8 pad74[0x58];
    SVECTOR vcc;        /* +0xCC */
} AcFlyCtl;

extern void func_80076E20(AcEntity *e, s32 sub, s32 a, void *fn);
extern void func_80079C0C(AcEntity *e);
extern s32 func_8004EBF8(AcMover *m);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);

#define F ((AcFlyCtl *)e)

s32 func_8007EE44(AcEntity *e)
{
    switch (((u8 *)&e->mover)[0]) {
    case 2:
    case 4:
    case 6:
        func_80076E20(e, 7, 1, func_80079C0C);
        F->h72 = 512;
        return 1;
    case 7:
        if (func_8004EBF8(&e->mover) != 0) {
            return 1;
        }
        func_80076E20(e, 7, 1, func_80079C0C);
        return 1;
    case 15:
    case 17:
    case 19:
        func_80076E20(e, 20, 1, func_80079C0C);
        F->h72 = 512;
        return 1;
    case 20:
        if (func_8004EBF8(&e->mover) != 0) {
            return 1;
        }
        func_80076E20(e, 20, 1, func_80079C0C);
        return 1;
    case 9:
        func_80076E20(e, 10, 1, func_80079C0C);
        F->h72 = 512;
        return 1;
    case 10:
        if (func_8004EBF8(&e->mover) != 0) {
            return 1;
        }
        func_80076E20(e, 10, 1, func_80079C0C);
        return 1;
    case 12:
        func_80076E20(e, 13, 1, func_80079C0C);
        F->h72 = 512;
        return 1;
    case 13:
        if (func_8004EBF8(&e->mover) != 0) {
            return 1;
        }
        func_80076E20(e, 13, 1, func_80079C0C);
        return 1;
    case 21:
        func_8004E98C(&e->mover, 0, 1, 512);
        ((u8 *)&e->mover)[2] = 4;
        F->vcc.vy = 0;
        F->vcc.vx = 0;
        return 1;
    }
    return 1;
}
