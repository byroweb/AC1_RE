#include "ac1/ac1.h"

extern u16 D_8004125E[];

typedef struct {
    u8 pad[0x148];
    u16 fire_flags;     /* +0x148 */
    u8 col[3];          /* +0x14A */
    u8 pad14d[0x0F];
    u32 flags;          /* +0x15C */
} AcFlash;

void func_8007A058(AcFlash *e)
{
    e->fire_flags |= 0xF;
    if (e->flags & D_8004125E[0]) {
        e->flags &= ~D_8004125E[0];
        e->col[0] = 60;
        e->col[1] = 40;
        e->col[2] = 20;
    } else {
        e->flags |= D_8004125E[0];
        e->col[0] = 180;
        e->col[1] = 160;
        e->col[2] = 140;
    }
}
