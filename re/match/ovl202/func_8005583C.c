#include "ac1/ac1.h"

typedef struct {
    s16 state;
    u8 pad02[0x1E];
    s32 f20;
    u8 pad24[4];
} Rec28;

extern Rec28 D_801A6548[];

void func_8005583C(void) {
    Rec28 *r = D_801A6548;
    u16 i;

    i = 32;
    do {
        r->state = 0;
        r->f20 = 0;
        r++;
    } while (--i);
}
