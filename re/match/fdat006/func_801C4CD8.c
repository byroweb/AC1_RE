#include "ac1/ac1.h"

extern void func_80076548(AcEntity *, s32, void *);
extern void func_80076578(AcEntity *, s32, s32, void *);
extern void func_80076658(AcEntity *, s32, s32, s32, s32, s32, void *);
extern void func_800767D8(AcEntity *, s32, s32, s32, s32, void *, void *);
extern void func_8004E98C(AcMover *, s32, s32, s32);
extern void func_80078598(AcEntity *);
extern u8 D_80076014[];
extern u8 D_800760DC[];
extern u8 D_800763F4[];
extern u8 D_801C61AC[];

void func_801C4CD8(AcEntity *e, s32 st)
{
    s32 m;

    if (e->faction & 0x10) {
        st = e->unk3a[2];
    }
    func_80078598(e);
    switch (st) {
    case 1:
        func_800767D8(e, 1, 1, 0, 1, D_801C61AC, D_800760DC);
        break;
    case 2:
        func_800767D8(e, 2, 1, 0, 1, D_801C61AC, D_800760DC);
        break;
    case 3:
        m = e->mover.mode;
        if (m < 5) {
            if (m >= 3) {
                func_80076548(e, 2, D_800763F4);
                func_8004E98C(&e->mover, 3, 1, 0);
                *(s16 *)((u8 *)e + 0xCE) = 20;
                *(s16 *)((u8 *)e + 0xD0) = 110;
                e->mover.unk66[0] = 2;
                break;
            }
        }
        func_80076658(e, 3, 1, 8, 20, 110, D_800763F4);
        break;
    case 4:
        m = e->mover.mode;
        if (m < 5) {
            if (m >= 3) {
                func_80076548(e, 3, D_800763F4);
                func_8004E98C(&e->mover, 4, 1, 0);
                *(s16 *)((u8 *)e + 0xCE) = -20;
                *(s16 *)((u8 *)e + 0xD0) = -110;
                e->mover.unk66[0] = 2;
                break;
            }
        }
        func_80076658(e, 4, 1, 8, -20, -110, D_800763F4);
        break;
    case 0:
        func_80076578(e, 0, 1, D_80076014);
        break;
    }
    if (!(e->faction & 0x10)) {
        e->unk3a[2] = st;
    }
}
