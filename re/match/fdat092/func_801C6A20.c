/* fdat092 0x801C6A20  size 188  permuter tier (tools/loop/permute.py): levers bytes &D_8019FB68 + E, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80078CFC(M2C_UNK *);                   /* extern */
M2C_UNK func_8008A80C();                            /* extern */
M2C_UNK func_8008C2FC(M2C_UNK, M2C_UNK *);          /* extern */
void *func_801C67A4(M2C_UNK);                       /* extern */
M2C_UNK func_801C6800(M2C_UNK);                     /* extern */
extern u16 D_8019F528;
extern M2C_UNK D_8019FB68;
extern s32 D_801CCBD8;
extern void *D_801D3B94;
extern M2C_UNK func_801C6880;
extern M2C_UNK func_801C69E4;

void func_801C6A20(void) {
    u8 temp_v1;

    func_8008A80C();
    if ((D_8019F528 == 0) && (D_801CCBD8 == 0)) {
        temp_v1 = M2C_FIELD(D_801D3B94, u8 *, 0x10);
        if (temp_v1 == 1) {
            D_801CCBD8 = (s32) temp_v1;
            func_80078CFC(&D_8019FB68);
            M2C_FIELD(func_801C67A4(4), M2C_UNK **, 0x5C) = &func_801C69E4;
            func_80078CFC(((M2C_UNK *)((u8 *)&D_8019FB68 + 0x2C)));
            func_801C67A4(5);
            func_8008C2FC(0x100, &func_801C6880);
            func_801C6800(5);
        }
    }
}
