#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

/* Toggle the current player's AI flag bits on e and set the 0x14A colour bytes. */
void func_80078B38(AcEntity *e) {
    e->fire_flags |= 0xF;
    if (e->flags & D_80039D18->flag_mask) {
        e->flags &= ~D_80039D18->flag_mask;
        e->unk14a[0] = 60;
        e->unk14a[1] = 40;
        e->unk14a[2] = 20;
    } else {
        e->flags |= D_80039D18->flag_mask;
        e->unk14a[0] = 180;
        e->unk14a[1] = 160;
        e->unk14a[2] = 140;
    }
}
