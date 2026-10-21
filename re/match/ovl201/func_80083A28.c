#include "ac1/ac1.h"
/* ovl201 0x80083A28 - place the text cursor after the name (at most the 8th cell) */
typedef struct NameCtl {
    u8 unk00[0x96];
    u8 len;                     /* +0x96 */
} NameCtl;

void func_80083A28(UiObj *o)
{
    u8 n;

    n = ((NameCtl *)o->w.list.src)->len;
    if (n == 8) {
        n = 7;
    }
    o->pos.vx = n * 16 + 8;
}
