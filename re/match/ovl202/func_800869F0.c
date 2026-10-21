#include "ac1/ac1.h"

extern s32 func_80014D78(s32 range);
extern Projectile *func_80086854(void *owner, AcPos *pos, u32 *flags);

/* one missile at pos, then n-1 more scattered by up to +-spread */
void func_800869F0(void *owner, AcPos *pos, s32 spread, s32 n, u32 *flags)
{
    AcPos at;
    u32 fl;
    s32 d;

    d = spread * 2;
    if (n != 0) {
        func_80086854(owner, pos, flags);
        if (n > 0) {
            fl = *flags | 0xF8001C00;
            do {
                at.x = pos->x + func_80014D78(d) - spread;
                at.y = pos->y + func_80014D78(d) - spread;
                n--;
                at.z = pos->z + func_80014D78(d) - spread;
                func_80086854(owner, &at, &fl);
            } while (n != 0);
        }
    }
}
