#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    s16 state;          /* +0x06 */
    u8 pad8[0x16];
    s16 owner;          /* +0x1E */
    u8 pad20[0x20];
} Func801C73F0Slot;     /* 0x40 */
extern Func801C73F0Slot D_801D0B68[];

s32 func_801C73F0(s32 id)
{
    Func801C73F0Slot *s;
    s32 i;
    s32 n;

    i = 512;
    n = 0;
    s = D_801D0B68;
    for (; i != 0; s++, i--) {
        if (s->state != -1 && s->owner == id) {
            n++;
        }
    }
    return n;
}
