/* exe 0x8001586C  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8001586C(s32 arg0) {
    return (((u32) (arg0 & 0xFF) >> 4) * 0xA) + (arg0 & 0xF);
}
