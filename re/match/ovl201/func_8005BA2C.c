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
extern void func_8005BF0C(UiObj *o);
extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);

/* Open a window box (x, y, w, h) in pool kind `kind`, starting from a centred sliver that 0x8005BF0C grows. */
UiObj *func_8005BA2C(s16 x, s16 y, s16 w, s16 h, s16 kind, s16 a, s16 b)
{
    UiObj *o;
    s16 cw;
    s16 ch;
    s16 s;

    o = ui_obj_alloc(kind);
    o->ctor = func_8005BA2C;
    o->update = func_8005BF0C;
    o->destroy = ui_obj_nop;
    o->render_type = 5;
    o->attached = 0;
    o->light = 0;
    o->has_xform = 0;
    ((BoxObj *)o)->b.a = a;
    ((BoxObj *)o)->b.b = b;
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
    ((BoxObj *)o)->b.tx = x;
    ((BoxObj *)o)->b.ty = y;
    ((BoxObj *)o)->b.tw = w;
    ((BoxObj *)o)->b.th = h;
    if (w > h) {
        s = h & ~1;
        ((BoxObj *)o)->b.x = x - (w - s) / 2 + w / 2;
        ((BoxObj *)o)->b.y = y + h / 2;
        ((BoxObj *)o)->b.w = w - s;
        ((BoxObj *)o)->b.h = 0;
    } else if (w < h) {
        s = w & ~1;
        ((BoxObj *)o)->b.x = x + w / 2;
        ((BoxObj *)o)->b.y = y - (h - s) / 2 + h / 2;
        ((BoxObj *)o)->b.w = 0;
        ((BoxObj *)o)->b.h = h - s;
    } else {
        ((BoxObj *)o)->b.x = x + w / 2;
        ((BoxObj *)o)->b.y = y + h / 2;
        ((BoxObj *)o)->b.w = 0;
        ((BoxObj *)o)->b.h = 0;
    }
    cw = ((BoxObj *)o)->b.w;
    ch = ((BoxObj *)o)->b.h;
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
