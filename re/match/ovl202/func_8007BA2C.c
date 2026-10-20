/* ovl202 0x8007BA2C  size 108  permuter tier (tools/loop/permute.py): levers bytes var_v1 += 0x30, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_80040FC4;
extern u16 D_80040FC8;
extern s16 D_80040FCA;
extern s16 D_80040FCC;
extern s16 D_80040FCE;
extern u16 D_800411C0;

void func_8007BA2C(void) {
    TargetSlot *var_v1;
    s32 var_a0;
    u8 *var_v0;

    var_v1 = g_target_list;
    var_a0 = 8;
    var_v0 = g_target_list->unk0c;
    do {
        var_v1->target = NULL;
        M2C_FIELD(var_v0, s16 *, 2) = 0;
        M2C_FIELD(var_v0, s16 *, 0) = 0;
        var_v0 += 0x30;
        var_a0 -= 1;
        var_v1 = (TargetSlot *)((u8 *)var_v1 + 0x30);
    } while (var_a0 != 0);
    g_target_head = NULL;
    D_80040FC4 = 0;
    D_80040FCE = 0;
    D_80040FCC = 0;
    D_80040FCA = 0;
    D_80040FC8 = D_800411C0;
}
