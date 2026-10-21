#include "ac1/ac1.h"

typedef struct {
    s16 state;
    u8 pad02[0x1E];
    s32 f20;
    u8 pad24[4];
} Rec28;

extern Rec28 D_801A6548[];

void func_8005586C(void) {
    Rec28 *r = D_801A6548;
    u16 i;

    i = 32;
    do {
        if (r->state != 0) {
            r->state = 1;
        }
        r++;
    } while (--i);
}
