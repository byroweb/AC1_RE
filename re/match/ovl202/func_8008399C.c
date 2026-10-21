#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

extern void func_80083928(Projectile *p);

void func_8008399C(Projectile *p, Vec3s *pos, s32 count, s8 a3, s32 step, s32 a5)
{
    p->free = 1;
    *(Vec3s *)p->dtint = *pos;
    M2C_FIELD(p, s8 *, 0x1F) = a3;
    M2C_FIELD(p, s8 *, 0x21) = 0;
    p->think = func_80083928;
    M2C_FIELD(p, s8 *, 0x1E) = step;
    M2C_FIELD(p, s8 *, 0x20) = a5;
    p->unk12 = (count - 1) * step + 1;
    func_80083928(p);
}
