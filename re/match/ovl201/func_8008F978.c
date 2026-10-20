/* ovl201 0x8008F978  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800AE30C();                            /* extern */
extern M2C_UNK D_800B8C98;
extern s8 D_801A2309;
extern s8 D_801A230A;
extern s8 D_801A230B;
extern s8 D_801A230C;
extern s8 D_801A230D;
extern s8 D_801A230E;
extern M2C_UNK *D_801A2310;
extern M2C_UNK *D_801A2314;
extern M2C_UNK func_8008C1F8;

void func_8008F978(void) {
    D_801A2309 = 1;
    D_801A230A = 7;
    D_801A2314 = &func_8008C1F8;
    D_801A230B = 0xA;
    D_801A230C = 0xE;
    D_801A230D = 0xD;
    D_801A230E = 6;
    D_801A2310 = &D_800B8C98;
    func_800AE30C();
}
