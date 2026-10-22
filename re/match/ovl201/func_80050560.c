/* ovl201 func_80050560: destroy every used object in the main UI pool.
 * Retail clears g_ui_pool[0].parent each time, not p->parent. */
#include "ac1/ac1.h"

void func_80050560(void)
{
    UiObj *p;

    for (p = g_ui_pool; p < &g_ui_pool[510]; p++) {
        if (p->used) {
            p->destroy(p);
            p->used = 0;
            p->kill = 0;
            g_ui_pool[0].parent = 0;
        }
    }
}
