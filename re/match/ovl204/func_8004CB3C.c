#include "ac1/ac1.h"
#include "ac1/ovl204_sound.h"

extern s32 func_8004CA24(SVECTOR *p, SVECTOR *l, s32 vol, s32 range, s32 maxd, VECTOR *d);
extern void func_8004D278(s32 id, s32 a, s32 l, s32 r, s32 pitch);

/* Play a positioned sound: left/right volume from the two ears, pitch from
 * the motion along the line of sight. */
s32 func_8004CB3C(s32 id, s32 a1, SVECTOR *p, SVECTOR *vel, s32 vol, s32 range, s32 maxd,
                  s32 pitch, s32 doppler) {
    VECTOR *d0 = (VECTOR *)0x1F8003F0;
    VECTOR *d1 = (VECTOR *)0x1F8003E0;
    s32 l;
    s32 r;

    id &= 0xFFF;
    l = func_8004CA24(p, &D_80041B90.pos, vol, range, maxd, d0);
    r = func_8004CA24(p, &D_80041B90.pos2, vol, range, maxd, d1);
    if (l == 0 && r == 0) {
        return 0;
    }
    if (l < r) {
        d0 = d1;
    }
    pitch -= (((d0->vx * vel->vx + d0->vy * vel->vy + d0->vz * vel->vz) / d0->pad) * doppler) >> 12;
    func_8004D278(id, a1, l, r, pitch);
    return 1;
}
