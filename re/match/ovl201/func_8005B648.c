#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

typedef struct {
    s16 x, y, w, h;             /* +0x7C current box */
    s16 a, b;                   /* +0x84 */
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

extern DrawSlot D_801BCE88[];
extern DrawSlot D_801D62E0[];
extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);

/* Static window box (x, y, w, h) in pool kind `kind`: sets both draw environments once. */
UiObj *func_8005B648(s16 x, s16 y, s16 w, s16 h, s16 kind, s16 a, s16 b)
{
    UiObj *o;
    s16 cw;
    s16 ch;

    o = ui_obj_alloc(kind);
    o->ctor = func_8005B648;
    o->update = ui_obj_nop2;
    o->destroy = ui_obj_nop;
    o->render_type = 1;
    o->attached = 0;
    o->light = 0;
    ((BoxObj *)o)->b.tx = 0;
    ((BoxObj *)o)->b.a = a;
    ((BoxObj *)o)->b.b = b;
    o->has_xform = 0;
    switch ((s16)(kind - 1)) {
    case 0:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 0;
        break;
    case 2:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 1;
        break;
    case 4:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 2;
        break;
    case 6:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 3;
        break;
    case 8:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 4;
        break;
    case 10:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 5;
        break;
    case 12:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 6;
        break;
    case 14:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 7;
        break;
    case 16:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 8;
        break;
    case 18:
        o->unk14 = 1406;
        ((BoxObj *)o)->b.buf = 9;
        break;
    }
    o->pos.vx = 0;
    o->pos.vy = 0;
    o->pos.vz = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    ((BoxObj *)o)->b.x = x;
    ((BoxObj *)o)->b.y = y;
    ((BoxObj *)o)->b.w = w;
    ((BoxObj *)o)->b.h = h;
    cw = ((BoxObj *)o)->b.w;
    ch = h;
    if (((BoxObj *)o)->b.x + cw > 320) {
        cw = 320 - ((BoxObj *)o)->b.x;
    }
    if (((BoxObj *)o)->b.y + ch > 240) {
        ch = 240 - ((BoxObj *)o)->b.y;
    }
    SetDefDrawEnv(&D_801BCE88[((BoxObj *)o)->b.buf].env, ((BoxObj *)o)->b.x, ((BoxObj *)o)->b.y, cw, ch);
    SetDefDrawEnv(&D_801D62E0[((BoxObj *)o)->b.buf].env, ((BoxObj *)o)->b.x, ((BoxObj *)o)->b.y + 240, cw, ch);
    D_801BCE88[((BoxObj *)o)->b.buf].env.ofs[0] = D_801BCE88[((BoxObj *)o)->b.buf].env.clip.x + (D_801BCE88[((BoxObj *)o)->b.buf].env.clip.w - 320) / 2;
    D_801BCE88[((BoxObj *)o)->b.buf].env.ofs[1] = D_801BCE88[((BoxObj *)o)->b.buf].env.clip.y + (D_801BCE88[((BoxObj *)o)->b.buf].env.clip.h - 240) / 2;
    D_801D62E0[((BoxObj *)o)->b.buf].env.ofs[0] = D_801D62E0[((BoxObj *)o)->b.buf].env.clip.x + (D_801D62E0[((BoxObj *)o)->b.buf].env.clip.w - 320) / 2;
    D_801D62E0[((BoxObj *)o)->b.buf].env.ofs[1] = D_801D62E0[((BoxObj *)o)->b.buf].env.clip.y + (D_801D62E0[((BoxObj *)o)->b.buf].env.clip.h - 240) / 2;
    return o;
}
