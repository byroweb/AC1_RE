/* ovl203 0x800553CC - ovl202 func_800559D0 for the 2-entry record table of mode 1 */
#include "ac1/ac1.h"

typedef struct {
    s16 state;
    u8 pad02[2];
    s16 f04;
    u8 pad06[8];
    s16 f0e;
    s16 f10;
    u8 pad12[0x16];
} Rec28;

extern Rec28 D_8019DF80[];

Rec28 *func_800553CC(void) {
    Rec28 *r = D_8019DF80;
    u16 i;

    i = 2;
    do {
        if (r->state == 0) {
            r->f04 = 0xFF;
            r->f0e = -1;
            r->f10 = 0xFF;
            return r;
        }
        r++;
    } while (--i);
    return 0;
}
