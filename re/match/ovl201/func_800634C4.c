#include "ac1/ac1.h"
/* ovl201 0x800634C4 - draw the three top-edge pieces of a window frame
 * (corners c[0..3]) with func_800646AC, then the rest with func_8006035C. */

typedef struct {
    s16 x, y, w, h;
} RECT16;

extern void func_800646AC(UiObj *o, SVECTOR *v, RECT16 *tw);
extern void func_8006035C(UiObj *o, SVECTOR *c);

void func_800634C4(UiObj *o, SVECTOR *c)
{
    SVECTOR v[4];
    RECT16 r;

    v[0] = c[0];
    v[0].vx += 8;
    v[0].vy += 24;
    v[1] = c[1];
    v[1].vx -= 8;
    v[1].vy += 24;
    v[2] = c[2];
    v[2].vy = v[0].vy + 8;
    v[2].vx += 8;
    v[3] = c[3];
    r.x = 200;
    r.y = 208;
    r.w = 8;
    r.h = 3;
    v[3].vy = v[1].vy + 8;
    v[3].vx -= 8;
    func_800646AC(o, v, &r);

    v[0] = c[0];
    v[0].vy += 24;
    v[1] = c[1];
    v[1].vx = c[0].vx + 8;
    v[1].vy += 24;
    v[2] = c[2];
    v[2].vy = v[0].vy + 8;
    v[3] = c[3];
    v[3].vx = c[0].vx + 8;
    r.x = 192;
    r.y = 208;
    r.w = 8;
    r.h = 8;
    v[3].vy = v[1].vy + 8;
    func_800646AC(o, v, &r);

    v[0] = c[0];
    v[0].vx = c[1].vx - 8;
    v[0].vy += 24;
    v[1] = c[1];
    v[1].vx = c[1].vx;
    v[1].vy += 24;
    v[2] = c[2];
    v[2].vx = v[0].vx;
    v[2].vy = v[0].vy + 8;
    v[3] = c[3];
    v[3].vx = v[1].vx;
    v[3].vy = v[1].vy + 8;
    r.x = 216;
    r.y = 208;
    r.w = 8;
    r.h = 8;
    func_800646AC(o, v, &r);
    func_8006035C(o, c);
}
