#include "ac1/ac1.h"
/* ovl201 0x800500EC - obj_find_by_handle continued: the next used, not-killed
 * object built by ctor after o; &g_ui_none when there is none. */

UiObj *func_800500EC(void *ctor, UiObj *o)
{
    for (o++; o < &g_ui_pool[510]; o++) {
        if (o->used && !o->kill && o->ctor == ctor) {
            return o;
        }
    }
    return &g_ui_none;
}
