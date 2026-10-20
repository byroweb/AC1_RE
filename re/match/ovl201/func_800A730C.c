/* ovl201 0x800A730C  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_800BDCCC;
extern s8 D_8017F4B1;
extern s8 D_8017F4B2;
extern s8 D_8017F4B8;
extern s8 D_8017F4B9;
extern s8 D_8017F4BA;
extern s8 D_8017F4BB;
extern s8 D_8017F4BC;
extern s8 D_8017F4BD;
extern s8 D_8017F4BE;
extern s8 D_8017F4BF;
extern M2C_UNK func_800A737C;

void func_800A730C(void *arg0) {
    D_8017F4B9 = 0x41;
    D_8017F4BA = 0x43;
    D_8017F4B1 = 0;
    D_8017F4B2 = 0;
    D_8017F4BB = 0;
    D_8017F4BC = 0;
    D_8017F4BD = 0;
    D_8017F4BE = 0;
    D_8017F4BF = 0;
    D_8017F4B8 = 0;
    D_800BDCCC = 0;
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A737C;
}
