#include "ac1/ac1.h"

typedef struct {
    s16 x, y;
    s16 pad4, pad6;
} Corner;

typedef struct {
    s16 u, v, w, h;
} TexRect;

extern void func_800646AC(UiObj *o, Corner *pt, TexRect *r);
extern void func_8006035C(UiObj *o, Corner *src);

void func_80060E10(UiObj *o, Corner *src)
{
    Corner pt[4];
    TexRect r;

    pt[0] = src[0];
    pt[0].x += 8;
    pt[0].y += 152;
    pt[1] = src[1];
    pt[1].x -= 8;
    pt[1].y += 152;
    pt[2] = src[2];
    pt[2].y = pt[0].y + 8;
    pt[2].x += 8;
    pt[3] = src[3];
    pt[3].y = pt[1].y + 8;
    pt[3].x -= 8;
    r.u = 200;
    r.v = 208;
    r.w = 8;
    r.h = 8;
    func_800646AC(o, pt, &r);

    pt[0] = src[0];
    pt[0].y += 152;
    pt[1] = src[1];
    pt[1].x = src[0].x + 8;
    pt[1].y += 152;
    pt[2] = src[2];
    pt[2].y = pt[0].y + 8;
    pt[3] = src[3];
    pt[3].x = src[0].x + 8;
    pt[3].y = pt[1].y + 8;
    r.u = 192;
    r.v = 208;
    r.w = 8;
    r.h = 8;
    func_800646AC(o, pt, &r);

    pt[0] = src[0];
    pt[0].x = src[1].x - 8;
    pt[0].y += 152;
    pt[1] = src[1];
    pt[1].x = src[1].x;
    pt[1].y += 152;
    pt[2] = src[2];
    pt[2].x = pt[0].x;
    pt[2].y = pt[0].y + 8;
    pt[3] = src[3];
    pt[3].x = pt[1].x;
    pt[3].y = pt[1].y + 8;
    r.u = 216;
    r.v = 208;
    r.w = 8;
    r.h = 8;
    func_800646AC(o, pt, &r);

    func_8006035C(o, src);
}
