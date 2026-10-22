#include "ac1/ac1.h"

extern void func_80076578(AcEntity *, s32, s32, void *);
extern void func_800766E0(AcEntity *, s32, s32, s32, s32, s32, void *);
extern void func_8007790C(AcEntity *, u8, s32, s32, void *, s32, void *);
extern void func_80078598(AcEntity *);
extern u8 D_80076014[];
extern u8 D_800764C4[];
extern void func_801C4C94(void);

void func_801C4D30(AcEntity *e, s32 st)
{
    if (e->faction & 0x10) {
        st = e->unk3a[2];
    }
    func_80078598(e);
    switch (st) {
    case 1:
        func_800766E0(e, 1, 1, 0, 0, 0, func_801C4C94);
        break;
    case 2:
        func_800766E0(e, 2, 1, 0, 0, 0, func_801C4C94);
        break;
    case 0:
        func_80076578(e, 0, 1, D_80076014);
        break;
    case 3:
        func_8007790C(e, e->mover.mode, 1, 0x400, (u8 *)e + 0xCC, -1, D_800764C4);
        break;
    }
    if (!(e->faction & 0x10)) {
        e->unk3a[2] = st;
    }
}
