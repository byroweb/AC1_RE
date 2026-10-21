/* ovl201 func_8005015C = obj_find_by_id: first used object in the main UI
 * pool whose u16 id matches, else &g_ui_none. */
#include "ac1/ac1.h"
UiObj *func_8005015C(s16 id)
{
    UiObj *p;

    for (p = g_ui_pool; p < &g_ui_pool[510]; p++) {
        if (p->used && p->id == id) {
            return p;
        }
    }
    return &g_ui_none;
}
