#include "ac1/ac1.h"
#include "ac1/ovl204_sound.h"

extern void func_80014B74(SVECTOR *pos, SVECTOR *ang, void *out);

void func_8004D004(SVECTOR *a, SVECTOR *b) {
    D_80041B90.pos = *a;
    func_80014B74(a, a + 1, D_80041B90.rot08);
    D_80041B90.pos2 = *b;
    func_80014B74(b, b + 1, D_80041B90.rot1c);
}
