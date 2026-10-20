/* ovl201 0x80068A20  size 416  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK MoveImage(M2C_UNK *, M2C_UNK, M2C_UNK);     /* extern */
s32 func_80050FC4(M2C_UNK);                         /* extern */
M2C_UNK func_8005DD38(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8005DE08(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
void func_80068BC0(UiObj *);                        /* extern */
M2C_UNK func_8006BF84(s32);                         /* extern */
M2C_UNK func_8006F964();                            /* extern */
M2C_UNK func_8006F9EC();                            /* extern */
s32 func_8006FA74(UiObj *);                         /* extern */
M2C_UNK func_8006FC04();                            /* extern */
s32 func_8006FC84(UiObj *);                         /* extern */
M2C_UNK func_8006FDA0(UiObj *);                     /* extern */
M2C_UNK func_8006FF90(UiObj *);                     /* extern */
M2C_UNK func_800703D0(UiObj *);                     /* extern */
extern s8 D_800B7E48;
extern s8 D_800B7E49;
extern s8 D_800B7E4A;
extern M2C_UNK D_800B7FCC;
extern M2C_UNK D_800B7FD4;
extern s32 D_801A2574;

UiObj *func_80068A20(void *arg0, s32 arg1) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_80068A20;
    temp_v0->update = func_80068BC0;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    temp_v0->w.raw[8] = 2;
    temp_v0->w.raw[0xA] = 1;
    temp_v0->w.raw[0x10] = 0;
    temp_v0->w.raw[0x12] = 0;
    M2C_FIELD(temp_v0, s16 *, 0x92) = 0;
    M2C_FIELD(temp_v0, s16 *, 0x94) = 0;
    temp_v0->w.raw[0x4E] = 0;
    M2C_FIELD(temp_v0, s32 *, 0x88) = func_80050FC4(0x800);
    M2C_FIELD(temp_v0, s32 *, 0xA0) = 0;
    M2C_FIELD(temp_v0, s32 *, 0xA4) = 0;
    M2C_FIELD(temp_v0, s32 *, 0x98) = func_8006FA74(temp_v0);
    M2C_FIELD(temp_v0, s32 *, 0x9C) = func_8006FC84(temp_v0);
    func_8006F964();
    func_8006F9EC();
    func_8006FC04();
    func_8006FDA0(temp_v0);
    func_800703D0(temp_v0);
    func_8006FF90(temp_v0);
    temp_v0->w.list.src = arg0;
    M2C_FIELD(temp_v0, s32 *, 0x80) = arg1;
    D_801A2574 = 2;
    D_800B7E4A = 0;
    D_800B7E49 = 0;
    D_800B7E48 = 0;
    temp_v0->w.raw[0x13] = 1;
    func_8005DD38(0x380, 0x80, 0xB);
    func_8005DE08(0x380, 0x98, 0xB);
    MoveImage(&D_800B7FCC, 0x39A, 0);
    MoveImage(&D_800B7FCC, 0x39A, 0x40);
    MoveImage(&D_800B7FD4, 0x3B0, 0);
    DrawSync(0);
    func_8006BF84(M2C_FIELD(temp_v0, s32 *, 0x88));
    return temp_v0;
}
