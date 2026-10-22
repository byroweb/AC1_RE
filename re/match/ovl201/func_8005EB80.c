#include "ac1/ac1.h"

extern u8 D_80031A58[];
extern u8 D_80039CA2;
extern s32 func_8007AA04(s32 slot);
extern void func_80050AF4(s32 a, s32 b);
extern void func_80051450(s32 a, s32 b);
extern void func_80074A18(void);

void func_8005EB80(u8 flag)
{
    u8 c;
    s32 v;

    c = D_80031A58[0];
    if (c == 0xFF) {
        v = func_8007AA04(0);
        if ((u8)v != c) {
            func_80050AF4(4, (v & 0xFF) + 3);
        }
    }
    c = D_80031A58[11];
    if (c == 0xFF) {
        v = func_8007AA04(11);
        if ((u8)v != c) {
            func_80050AF4(6, (v & 0xFF) + 6);
        }
    }
    c = D_80031A58[3];
    if (c == 0xFF) {
        v = func_8007AA04(3);
        if ((u8)v != c) {
            func_80050AF4(5, (v & 0xFF) + 38);
        }
    }
    c = D_80031A58[2];
    if (c == 0xFF) {
        v = func_8007AA04(2);
        if ((u8)v != c) {
            func_80050AF4(3, (v & 0xFF) + 54);
        }
    }
    c = D_80031A58[1];
    if (c == 0xFF) {
        v = func_8007AA04(1);
        if ((u8)v != c) {
            func_80050AF4(14, (v & 0xFF) + 64);
        }
    }
    c = D_80031A58[9];
    if (c == 0xFF) {
        v = func_8007AA04(9);
        if ((u8)v != c) {
            func_80050AF4(12, (v & 0xFF) + 124);
        }
    }
    c = D_80031A58[8];
    if (c == 0xFF) {
        v = func_8007AA04(8);
        if ((u8)v != c) {
            func_80050AF4(11, (v & 0xFF) + 143);
        }
    }
    c = D_80031A58[2];
    if (c == 0xFF) {
        v = func_8007AA04(2);
        if ((u8)v != c) {
            func_80051450(10, (v & 0xFF) + D_80039CA2 * 10);
        }
    }
    c = D_80031A58[0];
    if (c == 0xFF) {
        v = func_8007AA04(0);
        if ((u8)v != c) {
            func_80051450(7, (v & 0xFF) + D_80039CA2 * 3);
        }
    }
    c = D_80031A58[3];
    if (c == 0xFF) {
        v = func_8007AA04(3);
        if ((u8)v != c) {
            func_80051450(9, (v & 0xFF) + D_80039CA2 * 16);
        }
    }
    c = D_80031A58[11];
    if (c == 0xFF) {
        v = func_8007AA04(11);
        if ((u8)v != c) {
            func_80051450(8, (v & 0xFF) + D_80039CA2 * 32);
        }
    }
    c = D_80031A58[1];
    if (c == 0xFF) {
        v = func_8007AA04(1);
        if ((u8)v != c) {
            func_80051450(11, (u8)v);
        }
    }
    if (flag) {
        func_80074A18();
    }
}
