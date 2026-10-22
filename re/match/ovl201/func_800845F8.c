#include "ac1/ac1.h"
/* ovl201 0x800845F8 - retire the two rows of five child objects, then func_80084474 */
typedef struct UiPairWork {
    u8 pad7c[0x14];
    UiObj *a[6];                /* +0x90 */
    UiObj *b[5];                /* +0xA8 */
} UiPairWork;
void func_80084474(UiObj *o);

void func_800845F8(UiObj *o)
{
    UiPairWork *w;
    s16 i;

    w = (UiPairWork *)&o->w;
    for (i = 0; i < 5; i++) {
        w->a[i]->kill = 1;
        w->b[i]->kill = 1;
        w->a[i] = &g_ui_none;
        w->b[i] = &g_ui_none;
    }
    func_80084474(o);
}
