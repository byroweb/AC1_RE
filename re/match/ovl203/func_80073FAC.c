#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    u8 kind;                    /* +0x00 0x30 = damage class */
    u8 unk01;
    u16 damage;                 /* +0x02 */
} Ovl203Hit;

/* Damage of a hit: scaled by the current player's factor for its class, times 2/3. */
s32 func_80073FAC(Ovl203Hit *h) {
    s32 v;

    v = h->damage;
    switch (h->kind & 0x30) {
    case 0x10:
        v = (v * D_80039D18->dmg_scale[0]) >> 6;
        break;
    case 0x20:
        v = (v * D_80039D18->dmg_scale[1]) >> 6;
        break;
    }
    return (v * 2) / 3;
}
