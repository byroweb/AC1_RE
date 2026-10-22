#include "ac1/ac1.h"

extern s32 func_8004EBF8(AcMover *);
extern void func_80076E20(AcEntity *, s32, s32, void *);
extern u8 D_8007618C[];

s32 func_801C5B8C(AcEntity *e)
{
    switch (e->mover.mode) {
    case 2:
        func_80076E20(e, 3, 1, D_8007618C);
        *(s16 *)((u8 *)e + 0x72) = 0x100;
        return 1;
    case 3:
        if (func_8004EBF8(&e->mover) == 0) {
            func_80076E20(e, 3, 1, D_8007618C);
        }
        return 1;
    case 5:
        func_80076E20(e, 6, 1, D_8007618C);
        *(s16 *)((u8 *)e + 0x72) = 0x100;
        return 1;
    case 6:
        if (func_8004EBF8(&e->mover) == 0) {
            func_80076E20(e, 6, 1, D_8007618C);
        }
        return 1;
    }
    return 1;
}
