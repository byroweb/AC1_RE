/* fdat002 (17 twins): give a world object a trigger record (func_80072120(26)): update
 * func_801C6324, event hook func_801C5EF8, position copied from the object, rates
 * stats +2 / +0 and +4 / +0, size = the larger of the x and z extents, height = the y
 * extent, then func_800131D0 on the record's +0x1A block. */
#include "ac1/ac1.h"
typedef struct {
    void (*update)();   /* +0x00 */
    s32 (*event)();     /* +0x04 */
    u8 pad8[8];
    u8 b10;             /* +0x10 */
    u8 b11;             /* +0x11 */
    u8 rate_a;          /* +0x12 */
    u8 rate_b;          /* +0x13 */
    u8 pad14[2];
    s16 size;           /* +0x16 */
    s16 height;         /* +0x18 */
    u8 blk[8];          /* +0x1A */
    SVECTOR pos;        /* +0x22 */
} Func801C63F8Hit;
typedef struct {
    u16 div;            /* +0x00 */
    u16 a;              /* +0x02 */
    u16 b;              /* +0x04 */
} Func801C63F8Stats;
typedef struct {
    u8 pad0[0x10];
    SVECTOR pos;        /* +0x10 */
    s16 min[4];         /* +0x18 */
    s16 max[4];         /* +0x20 */
    u8 pad28[4];
    Func801C63F8Hit *on_hit; /* +0x2C */
    u8 pad30[8];
    Func801C63F8Stats *stats; /* +0x38 */
} Func801C63F8World;
extern Func801C63F8Hit *func_80072120(s32 kind);
extern void func_801C6324();
extern s32 func_801C5EF8();
extern void func_800131D0(u8 *blk, s32 rate, s32 a);
void func_801C63F8(Func801C63F8World *w)
{
    u8 unused[144];
    Func801C63F8Hit *h;
    s32 sx, sy, sz;

    h = func_80072120(26);
    w->on_hit = h;
    h->update = func_801C6324;
    h->event = func_801C5EF8;
    h->b11 = 0;
    h->b10 = 0;
    h->pos = w->pos;
    h->rate_a = w->stats->a / w->stats->div;
    h->rate_b = w->stats->b / w->stats->div;
    sx = w->max[0] - w->min[0];
    sy = w->max[1] - w->min[1];
    sz = w->max[2] - w->min[2];
    if (sx < sz)
        sx = sz;
    h->size = sx;
    h->height = sy;
    func_800131D0(h->blk, h->rate_a, w->stats->a);
}
