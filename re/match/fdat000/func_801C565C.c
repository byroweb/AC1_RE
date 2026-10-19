/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0033): set the
 * low nibble of fire_flags, then flip the AI flag bits named by the other entity's
 * ignore_mask and pick the matching colour triple (60,40,20 cleared, 180,160,140 set). */
#include "ac1/ac1.h"
void func_801C565C(AcEntity *e, AcEntity *src)
{
    u32 mask;

    e->fire_flags |= 0xF;
    mask = src->ignore_mask;
    if (e->flags & mask) {
        e->flags &= ~mask;
        e->unk14a[0] = 60;
        e->unk14a[1] = 40;
        e->unk14a[2] = 20;
    } else {
        e->flags |= mask;
        e->unk14a[0] = 180;
        e->unk14a[1] = 160;
        e->unk14a[2] = 140;
    }
}
