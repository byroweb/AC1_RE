/* ovl203 0x80055238 - ovl202 func_8005583C for the 2-entry record table of mode 1 */
#include "ac1/ac1.h"

typedef struct {
    s16 state;
    u8 pad02[0x1E];
    s32 f20;
    u8 pad24[4];
} Rec28;

extern Rec28 D_8019DF80[];

void func_80055238(void) {
    Rec28 *r = D_8019DF80;
    u16 i;

    i = 2;
    do {
        r->state = 0;
        r->f20 = 0;
        r++;
    } while (--i);
}
