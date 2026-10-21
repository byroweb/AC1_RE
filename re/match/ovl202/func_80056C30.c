#include "ac1/ac1.h"

typedef struct {
    u8 used;
    u8 pad01[0x17];
} Rec18;

extern Rec18 D_801AC8A4[];

Rec18 *func_80056C30(void) {
    Rec18 *r = D_801AC8A4;
    s32 i;

    i = 8;
    do {
        if (r->used == 0xFF) {
            return r;
        }
        r++;
    } while (--i);
    return 0;
}
