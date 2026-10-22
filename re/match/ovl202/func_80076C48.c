#include "ac1/ac1.h"

extern void func_8004DEAC(AcMover *, s32, s32 *, s32 *);
extern s32 func_8004E22C(AcMover *, void *, void *, s32 *, s32 *);
extern void func_8004E98C(AcMover *, s32, s32, s32);
extern void func_80075108(AcEntity *, s32 *);
extern void func_80076548(AcEntity *, s32, s32);

void func_80076C48(AcEntity *e, s32 a1, s32 a2, s32 a3)
{
    s32 sp18[2];
    s32 sp20[2];
    AcMover *m;
    s32 r;

    if (e->unk3a[3] != 0x11) {
        *(AcPos *)e->unk18 = *(AcPos *)e->unk10;
    }
    func_80076548(e, 0x11, a3);
    m = &e->mover;
    func_8004E98C(m, a1, a2, 0);
    r = func_8004E22C(m, e->unk18, e->unk10, sp18, sp20);
    func_80075108(e, sp18);
    func_8004DEAC(m, r, sp18, sp20);
}
