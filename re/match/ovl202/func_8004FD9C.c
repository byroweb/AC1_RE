#include "ac1/ac1.h"

extern SVECTOR D_8008D934;
extern void func_80084314(Projectile *);
extern void func_800501DC(Projectile *, SVECTOR *, SVECTOR *, SVECTOR *, s32, s32);

void func_8004FD9C(s32 arg0, s32 arg1) {
    Projectile *p;
    s32 i;
    u32 e;

    e = (u32)g_ac_entities & 0x7FFFFFFF;
    p = g_proj_pool;
    i = 0x100;
    do {
        if (p->free == 0 && p->think == func_80084314 && p->unk12 > 0
            && ((u32)p->owner & 0x7FFFFFFF) != e) {
            func_800501DC(p, &D_8008D934, &D_8008D934, &D_8008D934, arg0, arg1);
        }
        p++;
        i--;
    } while (i != 0);
}
