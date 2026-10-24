#include "ac1/ac1.h"

typedef struct {
    u8 st;
    u8 pad1[0x2B];
} Slot;
extern s32 func_80052338();
extern void func_800529F0();
extern void func_80078CFC();
extern void func_8008A80C();
extern s32 func_801C784C();
extern u16 D_8019F528;
extern Slot D_8019FB68[];
extern Slot D_8019FC9C[];
extern s32 D_801CD768;
extern s32 D_801CD76C;
extern s32 D_801CD770;

void func_801C78B0(void)
{
    s32 temp_v0;
    s32 temp_v0_2;
    s32 var_s1;
    s32 var_s2;
    s32 var_s3_2;
    s32 var_v1;
    Slot *var_a0;
    Slot *var_s0;
    Slot *var_s2_2;
    Slot *var_s3;

    func_8008A80C();
    if (D_8019F528 == 0) {
        if (D_801CD768 == 0x44C) {
            func_800529F0(0, 0, 0x3E);
            func_800529F0(0, 0, 0x3F);
        }
        if (D_801CD768 >= 0x462 && func_80052338() != 0) {
            D_801CD76C = 1;
        }
        if (D_801CD768 == 0x974) {
            func_800529F0(0, 0, 0x98);
            func_800529F0(0, 0, 0xA2);
        }
        if (D_801CD770 == 0 && D_801CD768 >= 0x98A) {
            if (func_80052338() != 0) {
                var_s3 = 0;
                var_s2 = 0x7FFFFFFF;
                D_801CD770 = 1;
                var_s0 = D_8019FC9C;
                var_s1 = 0;
                do {
                    if (var_s0->st == 0) {
                        temp_v0 = func_801C784C(var_s0);
                        if (temp_v0 > 0xF423F && temp_v0 < var_s2) {
                            var_s2 = temp_v0;
                            var_s3 = var_s0;
                        }
                    }
                    var_s1++;
                    var_s0++;
                } while (var_s1 < 7);
                var_a0 = D_8019FC9C;
                if (var_s3 != 0) {
                    var_a0 = var_s3;
                }
                func_80078CFC(var_a0);
            }
        }
        if (D_801CD768 <= 0xFFFF) {
            D_801CD768++;
        }
        var_v1 = 0;
        if (D_801CD76C != 0) {
            var_s0 = D_8019FB68;
            var_s1 = 0;
            do {
                if (var_s0->st == 1) {
                    var_v1++;
                }
                var_s1++;
                var_s0++;
            } while (var_s1 < 7);
            if (var_v1 < 3) {
                var_s3_2 = 0x7FFFFFFF;
                var_s2_2 = 0;
                var_s0 = D_8019FB68;
                var_s1 = 0;
                do {
                    if (var_s0->st == 0) {
                        temp_v0_2 = func_801C784C(var_s0);
                        if (temp_v0_2 > 0x17D783 && temp_v0_2 < var_s3_2) {
                            var_s3_2 = temp_v0_2;
                            var_s2_2 = var_s0;
                        }
                    }
                    var_s1++;
                    var_s0++;
                } while (var_s1 < 7);
                if (var_s2_2 != 0) {
                    func_80078CFC(var_s2_2);
                }
            }
        }
    }
}
