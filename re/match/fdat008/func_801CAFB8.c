#include "ac1/ac1.h"

extern void func_801C7234(void *, s32, s32, s32, void *);

s32 func_801CAFB8(u8 *o)
{
    switch (o[0x64]) {
    case 0:
    case 2:
    case 4:
    case 6:
    case 9:
    case 12:
    case 15:
    case 17:
    case 19:
        if (*(s16 *)(o + 0xCC) != 0 || *(s16 *)(o + 0xD0) != 0) {
            func_801C7234(o, 0, 0, 0x100, o + 0xCC);
            return 0;
        }
    }
    return 1;
}
