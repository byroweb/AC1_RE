#include "ac1/ac1.h"
/* ovl202 0x8006D45C - 1 when every part of a world structure is destroyed
 * (no on_hit handler left), else 0. */

s32 func_8006D45C(WorldStruct *ws)
{
    s32 i;
    u16 *ids = ws->parts;

    i = ws->n_parts;
    while (--i != -1) {
        ws = &g_world_structs[*ids++];
        if (ws->on_hit) {
            return 0;
        }
    }
    return 1;
}
