/* exe 0x80014D30  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 rand();                                         /* extern */

s32 func_80014D30(s32 arg0) {
    s32 temp_s0;

    temp_s0 = rand();
    return (s32) ((temp_s0 + rand()) * arg0) >> 0xF;
}
