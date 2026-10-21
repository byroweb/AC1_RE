/* ovl202 func_80089ECC = proj_alloc: first free slot of g_proj_pool; with
 * kind 1 and a full pool, steal the first slot running one of the four
 * short-lived effect thinks. */
#include "ac1/ac1.h"

extern void func_80085454(Projectile *);
extern void func_80085510(Projectile *);
extern void func_8008938C(Projectile *);
extern void func_80089B44(Projectile *);

Projectile *func_80089ECC(s32 kind)
{
    Projectile *p;
    s32 i;

    p = g_proj_pool;
    for (i = 256; i != 0; i--, p++) {
        if (p->free == -1) {
            return p;
        }
    }
    if (kind == 1) {
        p = g_proj_pool;
        for (i = 256; i != 0; i--, p++) {
            if (p->think == func_80085510 || p->think == func_80085454
                || p->think == func_8008938C || p->think == func_80089B44) {
                return p;
            }
        }
    }
    return &g_proj_none;
}
