#include "ac1_types.h"
u32 func_80066F68(u8 *p)
{
    u32 sum = 0;
    u32 i = 0;
    do {
        sum += *p++;
        i++;
    } while (i < 0xBE4);
    return sum;
}
