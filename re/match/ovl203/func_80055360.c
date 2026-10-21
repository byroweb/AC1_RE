/* ovl203 0x80055360 - ovl202 func_80055964 for the 2-entry record table of mode 1 */
#include "ac1/ac1.h"

typedef struct {
    s16 state;
    u8 pad02[0x26];
} Rec28;

extern Rec28 D_8019DF80[];
extern void func_800552A4(Rec28 *);

void func_80055360(void) {
    Rec28 *r = D_8019DF80;
    u16 i;

    i = 2;
    do {
        if (r->state == 1) {
            func_800552A4(r);
        }
        r++;
    } while (--i);
}
