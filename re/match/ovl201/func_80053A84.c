#include "ac1/ac1.h"
/* ovl201 0x80053A84 - pick the update handler from the object's mode byte ('D' or other) */
void func_80053AB4(UiObj *o);
void func_80055CE0(UiObj *o);

void func_80053A84(UiObj *o)
{
    if (o->w.raw[0xAA - 0x7C] == 'D') {
        o->update = func_80053AB4;
    } else {
        o->update = func_80055CE0;
    }
}
