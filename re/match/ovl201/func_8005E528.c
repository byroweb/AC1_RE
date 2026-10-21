#include "ac1/ac1.h"

typedef struct {
    void *src;          /* +0x7C */
    s32 a;              /* +0x80 */
    s16 h84;            /* +0x84 */
    s16 pad86;
    s32 b;              /* +0x88 */
    s16 n;              /* +0x8C */
    s16 pad8e[3];
    s16 x;              /* +0x94 */
    s16 y;              /* +0x96 */
    s16 w;              /* +0x98 */
    s16 h;              /* +0x9A */
} UiWin;

extern void func_8005BA2C(s16 x, s16 y, s16 w, s16 h, s16 n, s32 a, s32 b);
extern void func_8005E684(UiObj *o);

#define W ((UiWin *)&o->w)

UiObj *func_8005E528(void *src, s32 a, s32 b, s16 n, s16 x, s16 y, s16 w, s16 h)
{
    UiObj *o;

    o = ui_obj_alloc(0);
    o->ctor = func_8005E528;
    o->update = func_8005E684;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    W->src = src;
    W->a = a;
    W->b = b;
    W->n = n;
    W->h84 = 0;
    W->w = w;
    W->h = h;
    W->x = 160 - w / 2;
    W->y = 120 - h / 2;
    func_8005BA2C(x, y, w, h, n - 1, 1, 0);
    return o;
}
