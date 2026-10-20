/* ovl202 0x80053908  size 72  permuter tier (tools/loop/permute.py): levers index D_801A5F34 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800574D8(void *, void *);              /* extern */
extern void *D_801A5F30;

extern u8 D_801A5F34[];
void func_80053908(s32 arg0, void *arg1) {
    void *temp_a1;

    (*(void **)(D_801A5F34 + (((arg0 & 0xFFFF) * 4)))) = arg1;
    temp_a1 = arg1 + M2C_FIELD(arg1, s32 *, 8);
    D_801A5F30 = temp_a1;
    func_800574D8(temp_a1, temp_a1);
}
