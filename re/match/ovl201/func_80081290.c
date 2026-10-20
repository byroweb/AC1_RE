/* ovl201 0x80081290  size 140  variant sweep (tools/match/variants.py): old lever: bytes E + &D_800B85F4, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_8005CB94(s32, M2C_UNK, s16, M2C_UNK);    /* extern */
extern M2C_UNK D_800B85F4;

void func_80081290(void) {
    s16 temp_v1;
    s16 var_s0;
    void *temp_v0;

    var_s0 = 0;
    do {
        temp_v0 = func_8005CB94(*(((M2C_UNK *)((u8 *)&D_800B85F4 + (var_s0 * 4)))), 8, (s16) ((var_s0 * 0x10) + 4), 4);
        temp_v1 = var_s0 + 1;
        var_s0 = temp_v1;
        M2C_FIELD(temp_v0, s16 *, 0x14) = 3;
    } while (temp_v1 < 5);
}
