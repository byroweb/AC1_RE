/* ovl201 0x8009A7E8  size 84  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SetBackColor(M2C_UNK, M2C_UNK, M2C_UNK);    /* extern */
M2C_UNK SetGeomOffset(M2C_UNK, M2C_UNK);            /* extern */
M2C_UNK SetGeomScreen(M2C_UNK);                     /* extern */
extern s8 D_8018E468;
extern s8 D_8018E469;
extern s8 D_8018E46A;

void func_8009A7E8(void) {
    SetGeomScreen(0x100);
    SetGeomOffset(0xA0, 0x78);
    D_8018E468 = 0;
    D_8018E469 = 0;
    D_8018E46A = 0;
    SetBackColor(0, 0, 0);
}
