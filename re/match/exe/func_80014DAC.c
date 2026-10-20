/* exe 0x80014DAC  size 80  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 rand();                                         /* extern */

s32 func_80014DAC(s32 arg0) {
    s32 temp_s0;

    temp_s0 = rand();
    return (s32) (((temp_s0 + rand()) - 0x8000) * arg0) >> 0xF;
}
