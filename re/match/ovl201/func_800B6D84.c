#include "ac1/ac1.h"

extern s32 D_800CF360;          /* last math error code (33 EDOM, 34 ERANGE) */
extern s32 D_800CF364;
extern long DeliverEvent(unsigned long ev1, long ev2);

/* Math library error hook: remember the error and raise the matching event. */
s32 func_800B6D84(s32 err, s32 arg) {
    D_800CF360 = err;
    D_800CF364 = arg;
    switch (err) {
    case 33:
        DeliverEvent(0xF4000002, 0x301);
        break;
    case 34:
        DeliverEvent(0xF4000002, 0x302);
        break;
    }
    return 0;
}
