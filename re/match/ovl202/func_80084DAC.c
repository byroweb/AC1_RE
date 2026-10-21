#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 a;
    s16 b;
    s16 glow;
    u8 c;
    u8 d;
    u8 e;
    u8 col[5];
} MissileDef;

extern s32 func_80084B58(Projectile *, void *, void *, s32, s32);
extern void func_8004CC30(s32, s32, void *, s32, s32, s32, s32);
extern void func_80056B54(s32, void *, s32, s32, s32, void *, s32);
extern void func_8008399C(Projectile *, void *, s32, s32, s32, s32);
extern Projectile *func_8008589C(void *, void *, s32, s32, s32, s32);
extern MissileDef D_80090354[];

void func_80084DAC(Projectile *p)
{
    u8 pad[40];
    MissileDef *def;

    M2C_FIELD(p, u16 *, 0xC6) = M2C_FIELD(p, u16 *, 0xC6) == 0;
    def = &D_80090354[M2C_FIELD(p, u16 *, 0x2E)];
    if (func_80084B58(p, (u8 *)p + 0x1E, (u8 *)p + 0x26, def->a, def->b) != 0) {
kill:
        func_8004CC30(7, 0, p, 127, 12000, 14000, 0);
        M2C_FIELD(p, u32 *, 8) &= 0x7FFFFFFF;
        func_8008399C(p, p->dtint, def->c, def->d, def->e, 0);
    } else {
        if (p->unk12 == 0) {
            goto kill;
        }
        func_80056B54(0, p, 0, p->section, 0xFFFF, def->col, def->glow);
        func_8008589C(0, p, 35, 500, 0, p->section)->unk12 = 0;
    }
}
