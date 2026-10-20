/* exe 0x800165E4  size 148  permuter tier (tools/loop/permute.py): levers bytes temp_v0 + 4 + bytes E + &D_8004A2A4, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80015A08(void *, u16, M2C_UNK *);      /* extern */
s32 func_80015B24(M2C_UNK, s32);                    /* extern */
M2C_UNK func_80016328(M2C_UNK *, M2C_UNK, s32);     /* extern */
extern M2C_UNK D_8004A2A4;

void func_800165E4(s32 arg0, s32 arg1, M2C_UNK arg2) {
    M2C_UNK sp10;
    s32 *temp_v0;
    s32 temp_s0;
    u16 temp_a1;
    void *temp_v1;

    temp_v0 = ((M2C_UNK *)((u8 *)&D_8004A2A4 + ((arg0 & 0xFFFF) * 0xC)));
    temp_v1 = ((arg1 & 0xFFFF) * 2) + *temp_v0;
    temp_a1 = M2C_FIELD(temp_v1, u16 *, 0);
    temp_s0 = M2C_FIELD(temp_v1, u16 *, 2) - temp_a1;
    func_80015A08(((s32 *)((u8 *)temp_v0 + 4)), temp_a1, &sp10);
    do {
        func_80016328(&sp10, arg2, temp_s0);
    } while (func_80015B24(arg2, temp_s0) != 0);
}
