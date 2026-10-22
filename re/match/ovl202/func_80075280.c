#include "ac1/ac1.h"

typedef struct {
    u8 kind;
    u8 pad;
    u16 val;
} ScaleItem;

extern u8 D_80041214;
extern u8 D_80041215;

s32 func_80075280(ScaleItem *it)
{
    s32 k;
    s32 n;

    k = it->kind & 0x30;
    n = it->val;
    switch (k) {
    case 0x10:
        n = (n * D_80041214) >> 6;
        break;
    case 0x20:
        n = (n * D_80041215) >> 6;
        break;
    }
    return n * 2 / 3;
}
