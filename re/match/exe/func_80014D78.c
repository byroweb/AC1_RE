/* exe 0x80014D78  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 rand();                                         /* extern */

s32 func_80014D78(s32 arg0) {
    return (s32) (rand() * arg0) >> 0xF;
}
