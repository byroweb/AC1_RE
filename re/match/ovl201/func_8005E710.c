#include "ac1/ac1.h"

typedef struct {
    void *src;          /* +0x7C */
    s32 a;              /* +0x80 */
    s16 tick;           /* +0x84 */
    s16 pad86;
    s32 b;              /* +0x88 */
    s16 n;              /* +0x8C */
    s16 pad8e;
    UiObj *child;       /* +0x90 */
    s16 x;              /* +0x94 */
    s16 y;              /* +0x96 */
    s16 w;              /* +0x98 */
    s16 h;              /* +0x9A */
} UiWin;

extern s32 D_801A2574;
extern s32 D_801A2568;
extern s32 D_801A2550;
extern void func_8005C2D0(s16 x, s16 y, s16 w, s16 h, s16 n);
extern void func_8005E8D8(UiObj *o);

void func_8005E710(UiObj *o, s16 limit)
{
    UiWin *w;
    s32 fl;

    w = (UiWin *)&o->w;
    switch (D_801A2574) {
    case 0:
        limit = 120;
        break;
    case 2:
        limit = 60;
        break;
    case 3:
        limit = 30;
        break;
    }
    if (++w->tick > limit || (D_801A2568 & 0x600060)) {
        fl = D_801A2568;
        if (fl & 0x40) {
            D_801A2550 |= 0x40;
        }
        if (fl & 0x20) {
            D_801A2550 |= 0x20;
        }
        if (fl & 0x400000) {
            D_801A2550 |= 0x400000;
        }
        if (fl & 0x200000) {
            D_801A2550 |= 0x200000;
        }
        w->child->kill = 1;
        obj_find_by_id(w->n - 1)->kill = 1;
        func_8005C2D0(w->x, w->y, w->w, w->h, w->n - 1);
        o->update = func_8005E8D8;
    }
}
