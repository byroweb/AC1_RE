/* fdat010 0x801C4E58  size 164  permuter tier (tools/loop/permute.py): levers args func_800754D0 +arg0 proto, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800754D0(void *, M2C_UNK *);                       /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_801C65DC;

void func_801C4E58(void *arg0) {
    u8 temp_v1;

    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801C65DC) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        temp_v1 = M2C_FIELD(arg0, u8 *, 0x3C);
        if ((temp_v1 == 0) || (temp_v1 == 3)) {
            M2C_FIELD(arg0, s16 *, 0x7C) = 0;
            M2C_FIELD(arg0, s16 *, 0x76) = (s16) (rand() & 0xFFF);
            M2C_FIELD(arg0, s16 *, 0x78) = (s16) (rand() & 0xFFF);
            M2C_FIELD(arg0, s16 *, 0x7A) = (s16) (rand() & 0xFFF);
            return;
        }
        M2C_FIELD(arg0, s16 *, 0x7C) = -1;
    }
}
