#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    u8 unk00[0x12];
    u16 cost;                   /* +0x12 */
    u8 unk14[0x24 - 0x14];
    u8 flags;                   /* +0x24 0x20 = uses energy */
} Ovl203Weapon;

/* Pay a weapon's energy cost from the current player; 0 when it cannot. */
s32 func_8007B27C(Ovl203Weapon *w) {
    if (w->flags & 0x20) {
        if (D_80039D18->flags20 & 1) {
            return 0;
        }
        if (D_80039D18->energy < w->cost) {
            return 0;
        }
        D_80039D18->energy -= w->cost;
    }
    return 1;
}
