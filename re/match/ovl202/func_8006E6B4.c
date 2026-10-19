/* ovl202 func_8006E6B4 = world_section_find: index of the world structure
 * func_8006E380 picks, starting after `hint` (-1 = from the start), or -1.
 * The second argument (a position) passes through untouched in a1. */
#include "ac1/ac1.h"
extern WorldStruct *func_8006E380(WorldStruct *start);
s32 func_8006E6B4(s32 hint)
{
    WorldStruct *r;
    if (hint == -1) {
        r = func_8006E380(0);
    } else {
        r = func_8006E380(&g_world_structs[hint]);
    }
    if (r == 0) {
        return -1;
    }
    return r - g_world_structs;
}
