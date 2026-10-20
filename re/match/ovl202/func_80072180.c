/* ovl202 0x80072180  size 308  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80072180(void *arg0) {
    void *temp_a1;

    temp_a1 = M2C_FIELD(arg0, void **, 0x2C);
    M2C_FIELD(arg0, u16 *, 0x10) = (u16) (M2C_FIELD(arg0, u16 *, 0x10) + M2C_FIELD(temp_a1, u16 *, 8));
    M2C_FIELD(arg0, u16 *, 0x12) = (u16) (M2C_FIELD(arg0, u16 *, 0x12) + M2C_FIELD(temp_a1, u16 *, 0xA));
    M2C_FIELD(arg0, u16 *, 0x14) = (u16) (M2C_FIELD(arg0, u16 *, 0x14) + M2C_FIELD(temp_a1, u16 *, 0xC));
    M2C_FIELD(arg0, u16 *, 0) = (u16) (M2C_FIELD(arg0, u16 *, 0) + M2C_FIELD(temp_a1, u16 *, 8));
    M2C_FIELD(arg0, u16 *, 2) = (u16) (M2C_FIELD(arg0, u16 *, 2) + M2C_FIELD(temp_a1, u16 *, 0xA));
    M2C_FIELD(arg0, u16 *, 4) = (u16) (M2C_FIELD(arg0, u16 *, 4) + M2C_FIELD(temp_a1, u16 *, 0xC));
    M2C_FIELD(arg0, u16 *, 8) = (u16) (M2C_FIELD(arg0, u16 *, 8) + M2C_FIELD(temp_a1, u16 *, 8));
    M2C_FIELD(arg0, u16 *, 0xA) = (u16) (M2C_FIELD(arg0, u16 *, 0xA) + M2C_FIELD(temp_a1, u16 *, 0xA));
    M2C_FIELD(arg0, u16 *, 0xC) = (u16) (M2C_FIELD(arg0, u16 *, 0xC) + M2C_FIELD(temp_a1, u16 *, 0xC));
    M2C_FIELD(arg0, u16 *, 0x18) = (u16) (M2C_FIELD(arg0, u16 *, 0x18) + M2C_FIELD(temp_a1, u16 *, 8));
    M2C_FIELD(arg0, u16 *, 0x1A) = (u16) (M2C_FIELD(arg0, u16 *, 0x1A) + M2C_FIELD(temp_a1, u16 *, 0xA));
    M2C_FIELD(arg0, u16 *, 0x1C) = (u16) (M2C_FIELD(arg0, u16 *, 0x1C) + M2C_FIELD(temp_a1, u16 *, 0xC));
    M2C_FIELD(arg0, u16 *, 0x20) = (u16) (M2C_FIELD(arg0, u16 *, 0x20) + M2C_FIELD(temp_a1, u16 *, 8));
    M2C_FIELD(arg0, u16 *, 0x22) = (u16) (M2C_FIELD(arg0, u16 *, 0x22) + M2C_FIELD(temp_a1, u16 *, 0xA));
    M2C_FIELD(arg0, u16 *, 0x24) = (u16) (M2C_FIELD(arg0, u16 *, 0x24) + M2C_FIELD(temp_a1, u16 *, 0xC));
}
