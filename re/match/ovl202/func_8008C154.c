#include "ac1/ac1.h"
/* ovl202 0x8008C154 - mission heap bump allocator: return the cursor and
 * advance it by n bytes rounded up to a word. */

u8 *func_8008C154(s32 n)
{
    u8 *p = g_mission_ctl.heap_cur;

    g_mission_ctl.heap_cur = p + ((n + 3) & ~3);
    return p;
}
