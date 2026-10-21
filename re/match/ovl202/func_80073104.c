/* ovl202 0x80073104 - run every world struct's handler (on_hit record, fn at +0) */
#include "ac1/ac1.h"
typedef void (*WsFn)(WorldStruct *);

void func_80073104(void)
{
    WorldStruct *ws;
    s32 n;
    WsFn fn;

    n = g_world_struct_count;
    ws = g_world_structs;
    do {
        if (ws->on_hit != 0) {
            fn = *(WsFn *)ws->on_hit;
            if (fn != 0) {
                fn(ws);
            }
        }
        n -= 1;
        ws += 1;
    } while (n != 0);
}
