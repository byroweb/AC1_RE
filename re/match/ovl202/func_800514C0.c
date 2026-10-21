#include "ac1/ac1.h"

extern s32 D_8008DA1C[];
extern void func_800610A4(DVECTOR *, s32 *, s32, s32, s32);

void func_800514C0(s32 color, s32 x, s32 y, s32 n, ...) {
    DVECTOR pts[16];
    DVECTOR *p;
    s32 *ap;
    s32 i;
    s32 *c;

    ap = &n;
    c = &D_8008DA1C[color];
    p = pts;
    for (i = n; i != -1; i--) {
        p->vx = x + *++ap;
        p->vy = y + *++ap;
        p++;
    }
    func_800610A4(pts, c, n, 16, 1);
}
