/* ovl201 0x8008C120  size 216  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8008E260(UiObj *);                        /* extern */
void func_8009544C(UiObj *);                        /* extern */
extern void *D_8019C154;
extern void *D_8019C158;
extern void *D_8019C15C;
extern void *D_8019C160;
extern void *D_8019C178;
extern void *D_8019C17C;
extern void *D_8019C180;
extern void *D_8019C184;
extern M2C_UNK func_8008DD50;
extern M2C_UNK func_800953AC;

void func_8008C120(void) {
    UiObj *temp_s0;
    UiObj *temp_v0;

    temp_s0 = obj_find_by_handle(&func_8008DD50);
    temp_v0 = obj_find_by_handle(&func_800953AC);
    M2C_FIELD(D_8019C154, s16 *, 0x16) = 0x15;
    M2C_FIELD(D_8019C158, s16 *, 0x16) = 0x15;
    M2C_FIELD(D_8019C15C, s16 *, 0x16) = 0x15;
    M2C_FIELD(D_8019C160, s16 *, 0x16) = 0x15;
    M2C_FIELD(D_8019C178, s16 *, 0x16) = 0x14;
    M2C_FIELD(D_8019C17C, s16 *, 0x16) = 0x14;
    M2C_FIELD(D_8019C180, s16 *, 0x16) = 0x14;
    M2C_FIELD(D_8019C184, s16 *, 0x16) = 0x14;
    temp_s0->update = func_8008E260;
    temp_v0->light = 0x15;
    temp_v0->update = func_8009544C;
}
