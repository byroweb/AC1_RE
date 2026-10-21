#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u8 r, g, b, cd;
} Rgba;

typedef struct {
    s32 unk0;
    u16 *curve;
} CurveDef;

extern void func_80056B54(s32, void *, s32, s32, s32, Rgba *, s32);
extern Rgba D_8008FB48[];
extern CurveDef D_80090164[];

void func_8008523C(Projectile *p)
{
    CurveDef *c;

    if (M2C_FIELD(p, u8 *, 0x1C) == 0) {
        p->free = 0;
        c = &D_80090164[p->dtint[0]];
        if (p->unk1b != 7) {
            func_80056B54(0, p, 0, p->section, D_8008FB48[p->unk1b].cd, &D_8008FB48[p->unk1b],
                          (c->curve[p->dtint[2]] * p->ang[1]) >> 10);
        }
        p->dtint[2] += p->dtint[1];
        p->ang[0] += p->dtint[1];
    } else {
        M2C_FIELD(p, u8 *, 0x1C)--;
    }
}
