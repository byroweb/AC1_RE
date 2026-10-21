#include "ac1/ac1.h"

typedef struct {
    s16 state;
    u8 pad02[0x26];
} Rec28;

extern Rec28 D_801A6548[];
extern void func_800558A8(Rec28 *);

void func_80055964(void) {
    Rec28 *r = D_801A6548;
    u16 i;

    i = 32;
    do {
        if (r->state == 1) {
            func_800558A8(r);
        }
        r++;
    } while (--i);
}
