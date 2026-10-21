/* ovl202 func_8008792C: per-frame projectile pass.  Runs every live slot's
 * think, then counts down the lifetimes (+0x12) and frees expired slots;
 * +0x0C mirrors the free flag one frame late. */
#include "ac1/ac1.h"

void func_8008792C(void)
{
    Projectile *p;
    s32 i;

    p = g_proj_pool;
    for (i = 256; i != 0; p++, i--) {
        if (p->free >= 0 && p->unk0c >= 0 && p->think) {
            p->think(p);
        }
    }
    p = g_proj_pool;
    for (i = 256; i != 0; i--, p++) {
        if (p->unk0c >= 0 && p->free >= 0 && p->unk12 != -1) {
            if (--p->unk12 == -1) {
                p->free = -1;
            }
        }
        p->unk0c = p->free;
    }
}
