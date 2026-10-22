#include "ac1/ac1.h"

typedef struct {
    u8 st;
    u8 pad1[9];
    s16 kind;           /* +0x0A */
    u8 padC[0x20];
} Slot;
extern void func_8004C318();
extern void func_8004CC30();
extern s32 func_80052338();
extern void func_800529F0();
extern void func_8006D39C();
extern void func_80078CFC();
extern void func_800873F8();
extern void func_80087798();
extern void func_8008A80C();
extern void func_801C705C();
extern Slot D_8019FAB8[];
extern u8 D_8019FD4C;
extern u8 D_8019FD78[];
extern u16 D_8019F528;
extern u16 D_801A2818;
extern s32 D_801C761C;
extern s32 D_801C7620;
extern u8 D_801D5868[];
extern s16 D_801D5880, D_801D5882, D_801D5884, D_801D5888, D_801D588A, D_801D588C;

void func_801C70A0(void)
{
    struct {
        s8 ty;
        u8 pad1;
        s16 w;
        s8 c;
    } sp20;
    s16 sp28[3];
    s32 i;
    s32 n;
    s32 v;

    func_8008A80C();
    if (D_8019F528 == 0) {
        if (D_801C761C == 0) {
            i = 0;
            n = 0;
            for (; i < 6; i++) {
                if (D_8019FAB8[i + 7].kind != -1 && D_8019FAB8[i + 7].st == 3) {
                    n++;
                }
            }
            if (n >= 3) {
                func_80078CFC(D_8019FD78);
                v = 1;
                goto store;
            }
        } else if (D_801C761C == 1) {
            if (D_8019FD4C == 1) {
                func_800529F0(0, 0, 0x48);
                v = 2;
                goto store;
            }
        } else if (D_801C761C == 2) {
            i = 0;
            n = 0;
            for (; i < 9; i++) {
                if (D_8019FAB8[i + 7].kind != -1 && D_8019FAB8[i + 7].st == 3) {
                    n++;
                }
            }
            if (n >= 7) {
                func_801C705C();
            }
        } else if (func_80052338() != 0) {
            if (D_801C761C == 0x42) {
                sp20.ty = 0x17;
                sp20.w = 30000;
                sp20.c = 0x40;
                sp28[0] = (D_801D5880 + D_801D5888) >> 1;
                sp28[1] = (D_801D5882 + D_801D588A) >> 1;
                sp28[2] = (D_801D5884 + D_801D588C) >> 1;
                func_8004CC30(7, 0, sp28, 0x7F, 30000, 0x88B8, -20);
                func_800873F8(D_801D5868, D_801D5868 + 0x10, 0x32, 0x14, 0x3C, 0);
                func_80087798(0, sp28, &sp20, 0x10, 1);
                func_8006D39C(D_801D5868);
            } else if (D_801C761C >= 0x85 && D_801A2818 != 0) {
                if (D_801C7620 == 10) {
                    func_800529F0(0, 0, 0x68);
                    func_8004C318(0x80);
                }
                D_801C7620++;
            }
            v = D_801C761C + 1;
        store:
            D_801C761C = v;
        }
    }
}
