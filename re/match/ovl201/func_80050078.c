#include "ac1/ac1.h"
/* ovl201 0x80050078 obj_find_by_handle - first used, not-killed object built
 * by ctor; &g_ui_none when there is none. */

UiObj *obj_find_by_handle(void *ctor)
{
    UiObj *o;

    for (o = g_ui_pool; o < &g_ui_pool[510]; o++) {
        if (o->used && !o->kill && o->ctor == ctor) {
            return o;
        }
    }
    return &g_ui_none;
}
