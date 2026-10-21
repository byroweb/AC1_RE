#include "ac1/ac1.h"
/* ovl202 0x8006E7D4 - mark a world struct and its links in a bitset; a link with bit 15
 * set marks that struct and its own links too (func_8006E718) */
void func_8006E718(u32 *bits, WorldStruct *ws);



void func_8006E7D4(u32 *bits, WorldStruct *ws)
{
    s32 i;
    u16 *p;
    s32 n;
    s32 v;

    if (ws != 0) {
        v = ws - g_world_structs;
        bits[v >> 5] |= 1 << (v & 0x1F);
        p = *(u16 **)ws->unk28;
        n = ws->unk26;
        while (--n != -1) {
            v = *p++;
            if (v & 0x8000) {
                func_8006E718(bits, &g_world_structs[v & 0x1FF]);
            } else {
                v &= 0x1FF;
                bits[v >> 5] |= 1 << (v & 0x1F);
            }
        }
    }
}
