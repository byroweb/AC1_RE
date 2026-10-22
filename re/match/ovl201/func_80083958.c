#include "ac1/ac1.h"
/* ovl201 0x80083958 - place the grid cursor at the column / row bytes it points at (16-pixel cells) */
typedef struct CurWork {
    u8 *col;                    /* +0x7C */
    u8 *row;                    /* +0x80 */
} CurWork;

void func_80083958(UiObj *o)
{
    CurWork *w;
    s32 x;
    s32 y;

    w = (CurWork *)&o->w;
    x = *w->col * 16 + 8;
    y = *w->row * 16 + 4;
    o->pos.vx = x;
    o->pos.vy = y;
}
