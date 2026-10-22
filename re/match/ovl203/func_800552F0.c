/* ovl203 0x800552F0 - ovl202 func_800558F4 for the 2-entry record table of mode 1 */
#include "ac1/ac1.h"

typedef struct {
    s16 state;
    u8 pad02[0x26];
} Rec28;

extern Rec28 D_8019DF80[];
extern void func_800552A4(Rec28 *);

void func_800552F0(void) {
    Rec28 *r = D_8019DF80;
    s16 i;

    i = 1;
    do {
        if (r->state != 0) {
            func_800552A4(r);
        }
        r++;
    } while (--i != -1);
}
