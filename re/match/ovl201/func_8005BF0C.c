#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

typedef struct {
    s16 x, y, w, h;             /* +0x7C current box */
    s16 a, b;                   /* +0x84 passed on to func_8005B648 */
    s16 buf;                    /* +0x88 env slot */
    s16 tx, ty, tw, th;         /* +0x8A target box */
} BoxOpen;

typedef struct {
    u8 pad[0x7C];
    BoxOpen b;
} BoxObj;

typedef struct {
    DRAWENV env;
    u8 pad[0x64 - 0x5C];
} DrawSlot;

extern s32 D_801A2574;
extern DrawSlot D_801BCE88[];
extern DrawSlot D_801D62E0[];
extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);
extern void func_8005B648(s16 x, s16 y, s16 w, s16 h, s32 id, s32 a, s32 b);

/* Grow a box towards its target size each frame; when it arrives, hand over to func_8005B648. */
void func_8005BF0C(UiObj *o)
{
    BoxOpen *b = (BoxOpen *)&o->w;
    s16 dx;
    s16 dy;
    s16 w;
    s16 h;

    if (D_801A2574 == 0) {
        dx = 6;
        dy = 6;
    } else {
        dy = D_801A2574 * 6;
        dx = dy;
    }
    b->x -= dx;
    b->y -= dy;
    b->w += dx * 2;
    b->h += dy * 2;
    if (b->x < b->tx) {
        b->x = b->tx;
    }
    if (b->y < b->ty) {
        b->y = b->ty;
    }
    if (b->tw < b->w) {
        b->w = b->tw;
    }
    if (b->th < b->h) {
        b->h = b->th;
    }
    w = ((BoxObj *)o)->b.w;
    h = ((BoxObj *)o)->b.h;
    if (((BoxObj *)o)->b.x + w > 320) {
        w = 320 - ((BoxObj *)o)->b.x;
    }
    if (((BoxObj *)o)->b.y + h > 240) {
        h = 240 - ((BoxObj *)o)->b.y;
    }
    SetDefDrawEnv(&D_801BCE88[((BoxObj *)o)->b.buf].env, ((BoxObj *)o)->b.x, ((BoxObj *)o)->b.y, w, h);
    SetDefDrawEnv(&D_801D62E0[((BoxObj *)o)->b.buf].env, ((BoxObj *)o)->b.x, ((BoxObj *)o)->b.y + 240, w, h);
    D_801BCE88[((BoxObj *)o)->b.buf].env.ofs[0] = D_801BCE88[((BoxObj *)o)->b.buf].env.clip.x + (D_801BCE88[((BoxObj *)o)->b.buf].env.clip.w - 320) / 2;
    D_801BCE88[((BoxObj *)o)->b.buf].env.ofs[1] = D_801BCE88[((BoxObj *)o)->b.buf].env.clip.y + (D_801BCE88[((BoxObj *)o)->b.buf].env.clip.h - 240) / 2;
    D_801D62E0[((BoxObj *)o)->b.buf].env.ofs[0] = D_801D62E0[((BoxObj *)o)->b.buf].env.clip.x + (D_801D62E0[((BoxObj *)o)->b.buf].env.clip.w - 320) / 2;
    D_801D62E0[((BoxObj *)o)->b.buf].env.ofs[1] = D_801D62E0[((BoxObj *)o)->b.buf].env.clip.y + (D_801D62E0[((BoxObj *)o)->b.buf].env.clip.h - 240) / 2;
    if (b->tx == b->x && b->ty == b->y && b->tw == b->w && b->th == b->h) {
        o->kill = 1;
        func_8005B648(b->tx, b->ty, b->tw, b->th, (s16)o->id, b->a, b->b);
    }
}
