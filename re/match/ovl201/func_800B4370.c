/* ovl201 0x800B4370  size 40  zero-agent sweep (tools/loop/sweep.py): neighbour+imm draft of ovl201 0x80066F68 (ovl201/func_80066F68.c, similarity 0.900), 2026-10-20 */
#include "ac1_types.h"
u32 func_800B4370(u8 *p)
{
    u32 sum = 0;
    u32 i = 0;
    do {
        sum += *p++;
        i++;
    } while (i < 0x38E2);
    return sum;
}
