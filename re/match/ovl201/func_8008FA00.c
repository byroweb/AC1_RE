#include "ac1/ac1.h"
/* ovl201 0x8008FA00 - is part `id` of shop category `kind` the one equipped?
 * The category maps to an equipped-part byte at 0x80031A58; categories 6, 8
 * and 9 have no single slot and answer 0. */

extern u8 D_80031A58[];

s32 func_8008FA00(u8 kind, u16 id)
{
    u8 slot[10] = { 2, 0, 3, 11, 8, 9, 10, 1, 0, 0 };

    if (kind == 6 || (u8)(kind - 8) < 2) {
        return 0;
    }
    return D_80031A58[slot[kind]] == id;
}
