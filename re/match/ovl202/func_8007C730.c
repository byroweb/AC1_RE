#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x12];
    u16 cost;       /* +0x12 */
    u8 pad14[0x10];
    u8 flags;       /* +0x24 */
} CostObj;

extern u16 D_80040FBE;
extern u16 D_80040FC0;

s32 func_8007C730(CostObj *o)
{
    if (!(o->flags & 0x20)) {
        return 1;
    }
    if (D_80040FC0 & 1) {
        return 0;
    }
    if (D_80040FBE >= o->cost) {
        D_80040FBE -= o->cost;
        return 1;
    }
    return 0;
}
