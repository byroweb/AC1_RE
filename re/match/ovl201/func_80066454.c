#include "ac1/ac1.h"
/* ovl201 0x80066454 - draw_string: draw the zero-terminated string o->res
 * glyph by glyph (func_80066614) in the font size o->unk10[0] (16, 8, 6 or 4;
 * 16 advances 8), then queue the draw mode (func_80066898). */

extern void func_80066614(UiObj *o, s16 x, s16 y, s16 w, s16 h, s32 u, s16 v);
extern void func_80066898(UiObj *o);

void func_80066454(UiObj *o)
{
    u8 *s;
    u8 size;
    u8 adv;
    s16 x;
    s16 y;
    s32 c;
    s32 u;
    s16 h;
    s16 w;
    s16 v;

    size = o->unk10[0];
    s = o->res;
    adv = 8;
    if (size != 16) {
        adv = size;
    }
    x = o->pos.vx;
    y = o->pos.vy;
    while (*s) {
        c = *s - 32;
        u = (c & 31) * 8;
        switch (size) {
        case 16:
            v = c / 32 * 16;
            h = 16;
            w = 8;
            break;
        case 8:
            v = c / 32 * 8 + 48;
            w = 8;
            h = 8;
            break;
        case 6:
            v = c / 32 * 8 + 72;
            w = 6;
            h = 8;
            break;
        case 4:
            v = c / 32 * 8 + 96;
            w = 4;
            h = 8;
            break;
        }
        s++;
        func_80066614(o, x, y, w, h, u, v);
        x += adv;
    }
    func_80066898(o);
}
