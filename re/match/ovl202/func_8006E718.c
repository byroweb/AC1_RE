#include "ac1/ac1.h"
/* ovl202 0x8006E718 - mark a world struct and its linked structs (u16 list at +0x28, count at +0x26) in a bitset */

void func_8006E718(u32 *bits, WorldStruct *ws)
{
    s32 i;
    u16 *p;
    s32 n;

    if (ws != 0) {
        i = ws - g_world_structs;
        p = *(u16 **)ws->unk28;
        n = ws->unk26;
        do {
            n--;
            bits[i >> 5] |= 1 << (i & 0x1F);
            i = *p++ & 0x1FF;
        } while (n != -1);
    }
}
