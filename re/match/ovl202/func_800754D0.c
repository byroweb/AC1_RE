#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014D78();
M2C_UNK func_8004CC30();
M2C_UNK func_80086854();
M2C_UNK func_800869F0();
M2C_UNK func_80086AD8();
M2C_UNK func_80086DDC();
M2C_UNK func_800873F8();
s32 func_800754D0(void *arg0, void *arg1) {
    SVECTOR p0;
    SVECTOR p1;
    s16 temp_a0;
    s16 temp_v0_2;
    s16 temp_v1;
    u8 temp_v0;
    temp_v0 = (*(u8 *)((s8 *)(arg0) + (0x40)));
    if (temp_v0 == 0) {
        p0.vx = (*(u16 *)((s8 *)(arg0) + (8)));
        p0.vy = (*(u16 *)((s8 *)(arg0) + (0xA))) - ((u16) (*(u16 *)((s8 *)(arg0) + (0x6C))) >> 1);
        p0.vz = (*(u16 *)((s8 *)(arg0) + (0xC)));
        temp_v0_2 = (*(s16 *)((s8 *)(arg1) + (0xC)));
        if (temp_v0_2 != 0) {
            func_80086AD8(0, &p0, (*(s16 *)((s8 *)(arg1) + (8))), (*(s16 *)((s8 *)(arg1) + (0xA))), (s32) temp_v0_2);
        }
        func_800869F0(0, &p0, (*(u16 *)((s8 *)(arg0) + (0x6A))) * 2, (*(s16 *)((s8 *)(arg1) + (0xE))), arg1);
        func_800873F8(arg0, &p0, (*(s16 *)((s8 *)(arg1) + (0x14))), (*(s16 *)((s8 *)(arg1) + (0x16))), (s32) (*(s16 *)((s8 *)(arg1) + (0x18))), 0);
        func_8004CC30((*(s16 *)((s8 *)(arg1) + (0x10))), 0, &p0, (*(u8 *)((s8 *)(arg1) + (0x12))), 0x4E20, 0x4650, (s32) (*(s8 *)((s8 *)(arg1) + (0x13))));
        temp_a0 = (*(s16 *)((s8 *)(arg0) + (0x2E)));
        temp_v1 = (s16) (*(u16 *)((s8 *)(arg0) + (0xA)));
        if ((temp_v1 >= temp_a0) && (temp_a0 >= (temp_v1 - (*(u16 *)((s8 *)(arg0) + (0x2A)))))) {
            p0.vy = temp_a0;
            func_80086DDC(0, &p0, (*(u16 *)((s8 *)(arg0) + (0x6A))), (*(s16 *)((s8 *)(arg1) + (0xE))), 0x28, ((u32) (*(u32 *)((s8 *)(arg1) + (0))) >> 0x11) & 0x3FF, ((u32) (*(u32 *)((s8 *)(arg1) + (0))) >> 0xF) & 0xFFC, (u32) ((*(u32 *)((s8 *)(arg1) + (0))) << 5) >> 0x17, (*(u16 *)((s8 *)(arg1) + (2))) & 0x7FE, 0x40);
        }
        return 1;
    }
    if (!(temp_v0 & 1)) {
        p1.vx = ((*(u16 *)((s8 *)(arg0) + (8))) + func_80014D78((*(u16 *)((s8 *)(arg0) + (0x6A))) * 2)) - (*(u16 *)((s8 *)(arg0) + (0x6A)));
        p1.vz = ((*(u16 *)((s8 *)(arg0) + (0xC))) + func_80014D78((*(u16 *)((s8 *)(arg0) + (0x6A))) * 2)) - (*(u16 *)((s8 *)(arg0) + (0x6A)));
        p1.vy = (*(u16 *)((s8 *)(arg0) + (0xA))) - func_80014D78((*(u16 *)((s8 *)(arg0) + (0x6C))));
        func_80086854(0, &p1, arg1 + 4);
        func_800873F8(arg0, &p1, (*(s16 *)((s8 *)(arg1) + (0x1E))), (*(s16 *)((s8 *)(arg1) + (0x20))), (s32) (*(s16 *)((s8 *)(arg1) + (0x22))), 1);
        if (!((*(u8 *)((s8 *)(arg0) + (0x40))) & 3)) {
            func_8004CC30((*(s16 *)((s8 *)(arg1) + (0x1A))), 0, &p1, (*(u8 *)((s8 *)(arg1) + (0x1C))), 0x3E80, 0x4650, (s32) (*(s8 *)((s8 *)(arg1) + (0x1D))));
        }
    }
    (*(u8 *)((s8 *)(arg0) + (0x40))) = (u8) ((*(u8 *)((s8 *)(arg0) + (0x40))) - 1);
    return 0;
}
