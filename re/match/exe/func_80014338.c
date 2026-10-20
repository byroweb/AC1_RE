/* exe 0x80014338  size 128  variant sweep (tools/match/variants.py): args lever VectorNormalSS +arg0, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK VectorNormalSS(s32, void *);                     /* extern */

void func_80014338(s32 arg0, s32 arg1, void *arg2) {
    VectorNormalSS(arg0, arg2);
    M2C_FIELD(arg2, s16 *, 0) = (s16) ((s32) (M2C_FIELD(arg2, s16 *, 0) * arg1) >> 0xC);
    M2C_FIELD(arg2, s16 *, 2) = (s16) ((s32) (M2C_FIELD(arg2, s16 *, 2) * arg1) >> 0xC);
    M2C_FIELD(arg2, s16 *, 4) = (s16) ((s32) (M2C_FIELD(arg2, s16 *, 4) * arg1) >> 0xC);
}
