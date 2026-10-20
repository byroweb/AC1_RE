/* ovl201 0x8006DF7C  size 300  zero-agent sweep (tools/loop/sweep.py): m2c ctx-regvars draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8006F220(void *);                      /* extern */
M2C_UNK func_8006F55C(void *, void *);              /* extern */
extern s32 D_801A2568;
extern s32 D_801A256C;

void func_8006DF7C(void *arg0) {
    u16 temp_v1;
    void *temp_a0;
    void *temp_a1;
    void *var_s0;
    void *var_s1;

    var_s0 = arg0;
    temp_a0 = M2C_FIELD(var_s0, void **, 0x98);
    temp_v1 = M2C_FIELD(temp_a0, u16 *, 0x7E);
    temp_a1 = M2C_FIELD(var_s0, void **, 0x9C);
    var_s1 = var_s0 + 0x7C;
    if (((u32) (temp_v1 - 0xAC) < 0x80U) && ((u32) (M2C_FIELD(temp_a0, u16 *, 0x80) - 0x24) < 0x80U)) {
        if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40) && (M2C_FIELD(var_s0, u8 *, 0x8E) == 0)) {
            M2C_FIELD(var_s0, s16 *, 0x92) = (s16) (M2C_FIELD(temp_a1, u16 *, 0x7C) + ((s32) ((s16) temp_v1 - 0xAC) / (s32) M2C_FIELD(var_s0, u8 *, 0x84)));
            M2C_FIELD(var_s0, s16 *, 0x94) = (s16) (M2C_FIELD(temp_a1, u16 *, 0x7E) + ((s32) ((s16) M2C_FIELD(temp_a0, u16 *, 0x80) - 0x24) / (s32) M2C_FIELD(var_s0, u8 *, 0x84)));
            func_8006F55C(temp_a0, temp_a1);
            func_8006F220(var_s0);
        }
        if (D_801A2568 & 0x20) {
            goto block_7;
        }
    } else {
block_7:
        if (D_801A2568 & 0x20) {
            M2C_FIELD(var_s1, s8 *, 0x12) = 0;
        }
    }
}
