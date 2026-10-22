/* fdat056 0x801C743C  size 264  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_801C739C(M2C_UNK);                         /* extern */
s32 func_801C73F0(M2C_UNK);                         /* extern */
extern s32 D_801CD7E8;
extern s32 D_801CD7EC;
extern s32 D_801CD7F0;
extern s32 D_801CD7F4;
extern s32 D_801CD7F8;
extern s32 D_801CD7FC;

void func_801C743C(void) {
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_s0_3;
    s32 temp_s1;
    s32 temp_s1_2;
    s32 temp_s2;
    s32 temp_s3;

    temp_s0 = func_801C739C(4);
    temp_s1 = func_801C739C(5);
    D_801CD7E8 = temp_s0 + temp_s1 + func_801C739C(6);
    temp_s0_2 = func_801C739C(0x15);
    D_801CD7EC = temp_s0_2 + func_801C739C(0x16);
    temp_s0_3 = func_801C73F0(0x23);
    temp_s3 = func_801C73F0(0x24);
    temp_s2 = func_801C73F0(0x25);
    temp_s1_2 = func_801C73F0(0x26);
    D_801CD7F0 = temp_s0_3 + temp_s3 + temp_s2 + temp_s1_2 + func_801C73F0(0x27);
    D_801CD7F4 = func_801C73F0(0x29);
    D_801CD7F8 = func_801C73F0(0x2A);
    D_801CD7FC = func_801C73F0(0x2B);
}
