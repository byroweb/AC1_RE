#include "ac1_types.h"
typedef struct {
    u8 pad[0x40];
} Func8006E6B4Ent;
extern Func8006E6B4Ent D_801D0B68[];
extern Func8006E6B4Ent *func_8006E380(Func8006E6B4Ent *start);
s32 func_8006E6B4(s32 idx)
{
    Func8006E6B4Ent *r;
    if (idx == -1) {
        r = func_8006E380(0);
    } else {
        r = func_8006E380(&D_801D0B68[idx]);
    }
    if (r == 0) {
        return -1;
    }
    return r - D_801D0B68;
}
