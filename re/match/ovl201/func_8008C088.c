/* ovl201 0x8008C088  size 152  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8008D560(UiObj *);                        /* extern */
void func_800951B0(UiObj *);                        /* extern */
extern void *D_8019C14C;
extern void *D_8019C150;
extern void *D_8019C170;
extern void *D_8019C174;
extern M2C_UNK func_8008C3EC;
extern M2C_UNK func_80095130;

void func_8008C088(void) {
    UiObj *temp_s0;
    UiObj *temp_v0;

    temp_s0 = obj_find_by_handle(&func_80095130);
    temp_v0 = obj_find_by_handle(&func_8008C3EC);
    M2C_FIELD(D_8019C14C, s16 *, 0x16) = 0x15;
    M2C_FIELD(D_8019C150, s16 *, 0x16) = 0x15;
    M2C_FIELD(D_8019C170, s16 *, 0x16) = 0x14;
    M2C_FIELD(D_8019C174, s16 *, 0x16) = 0x14;
    temp_v0->update = func_8008D560;
    temp_s0->light = 0x15;
    temp_s0->update = func_800951B0;
}
