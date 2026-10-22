/* fdat024 0x801C6E14  size 108  permuter tier (tools/loop/permute.py): levers etype D_801D3368 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8008A80C();                            /* extern */
M2C_UNK func_801C6C1C(M2C_UNK *);                   /* extern */
extern s32 D_801A5DC4;
extern u8 D_801D3368;

void func_801C6E14(void) {
    func_8008A80C();
    if ((D_801A5DC4 & 0x1F) == 0xF) {
        func_801C6C1C(&D_801D3368);
        func_801C6C1C(&D_801D3368 + 0xC0);
        func_801C6C1C(&D_801D3368 + 0x100);
        func_801C6C1C(&D_801D3368 + 0x40);
        func_801C6C1C(&D_801D3368 + 0x80);
    }
}
