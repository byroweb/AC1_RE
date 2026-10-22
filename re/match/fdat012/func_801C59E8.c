#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void func_800765E8(AcEntity *, s32, s32, void *);
extern void func_800775B8(AcEntity *, s32, s32, s32, void *, void *);
extern void func_80078598(AcEntity *);
extern u8 D_80076078[];
extern void func_801C5600();

void func_801C59E8(AcEntity *e, s32 st)
{
    if (e->faction & 0x10) {
        st = e->unk3a[2];
    }
    func_80078598(e);
    switch (st) {
    case 0:
        func_800765E8(e, 0, 1, D_80076078);
        break;
    case 1:
        if (e->mover.mode == 1) {
            func_800775B8(e, 1, 1, 512, (u8 *)e + 0xCC, func_801C5600);
        } else {
            M2C_FIELD(e, s16 *, 0xCE) = -64;
            M2C_FIELD(e, s16 *, 0xD2) = 0;
            M2C_FIELD(e, s16 *, 0xD0) = 0;
            M2C_FIELD(e, s16 *, 0xCC) = 0;
            func_800775B8(e, 1, 1, 512, (u8 *)e + 0xCC, func_801C5600);
        }
        break;
    case 2:
        func_800775B8(e, 2, 1, 512, (u8 *)e + 0xCC, func_801C5600);
        break;
    case 3:
    case 10:
        func_800775B8(e, 3, 1, 512, (u8 *)e + 0xCC, func_801C5600);
        break;
    case 4:
        func_800775B8(e, 4, 1, 512, (u8 *)e + 0xCC, func_801C5600);
        break;
    case 5:
    case 8:
        func_800775B8(e, 5, 1, 512, (u8 *)e + 0xCC, func_801C5600);
        break;
    case 6:
    case 9:
        func_800775B8(e, 6, 1, 512, (u8 *)e + 0xCC, func_801C5600);
        break;
    case 7:
        if (e->mover.mode == 1) {
            if (M2C_FIELD(e, s16 *, 0x6E) == 0) {
                e->faction &= 0xFFEF;
                func_800765E8(e, 0, 1, D_80076078);
                st = 0;
                break;
            }
        }
        func_800765E8(e, 1, -1, D_80076078);
        break;
    case 0x81:
        func_800775B8(e, e->mover.mode, 0, 512, (u8 *)e + 0xCC, func_801C5600);
        break;
    }
    if (!(e->faction & 0x10)) {
        e->unk3a[2] = st;
    }
}
