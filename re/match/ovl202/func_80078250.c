#include "ac1/ac1.h"

extern s32 func_800745D0(AcEntity *e, SVECTOR *out, s32 a, s32 b, s32 c, s32 d);
extern void func_80086C9C(s32 a, SVECTOR *v, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h, s32 i, s32 j, s32 k);

void func_80078250(AcEntity *e, s32 a, s32 b, s32 c, s32 p, s32 q, s32 r, s32 s)
{
    SVECTOR v;

    if (e->unk60 == 0) {
        return;
    }
    if (func_800745D0(e, &v, a, b, 1, c) == 0) {
        return;
    }
    v.vy = *(u16 *)((u8 *)e + 0x2E);
    func_80086C9C(0, &v, 40, p, q, p >> 2, q >> 2, -p >> 5, -q >> 3, q >> 2, r, s);
}
