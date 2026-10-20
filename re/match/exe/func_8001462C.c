/* exe 0x8001462C  size 80  variant sweep (tools/match/variants.py): args lever: args func_800145E4 +arg0 noproto, from the ctx-knr draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800145E4();                    /* extern */

void func_8001462C(s32 arg0, void *arg1) {
    func_800145E4(arg0, M2C_FIELD(arg1, s16 *, 0));
    func_800145E4(arg0 + 6, M2C_FIELD(arg1, s16 *, 2));
    func_800145E4(arg0 + 0xC, M2C_FIELD(arg1, s16 *, 4));
}
