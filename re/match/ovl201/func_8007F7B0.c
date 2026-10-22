#include "ac1/ac1.h"
/* ovl201 0x8007F7B0 - make one slider knob (chan 'R'/'G'/else B) of the paint colour editor:
 * sprite 0x800B84D8 at x = 224 + the component value, row 185/193/201; update 0x8007F9C4 */
typedef struct {
    u8 pad7c[9];
    u8 part;                    /* +0x85 */
    u8 row;                     /* +0x86 */
} PaintWork;

typedef struct {
    UiObj *owner;               /* +0x7C */
    u8 chan;                    /* +0x80 'R', 'G' or 'B' */
} KnobWork;

extern u8 D_80031A64[4][4][3];
#define W ((PaintWork *)&owner->w)
extern u8 D_800B84D8[];
extern void func_8007F9C4(UiObj *o);

UiObj *func_8007F7B0(u8 chan, UiObj *owner)
{
    UiObj *o;

    u8 part;
    u8 row;
    s32 v;

    o = ui_obj_alloc(2);
    o->ctor = func_8007F7B0;
    o->update = func_8007F9C4;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 21;
    o->unk14 = 2;
    o->res = D_800B84D8;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;

    do switch (W->part) {
    case 'G':
    case 'H':
        part = 0;
        break;
    case 'C':
        part = 1;
        break;
    case 'A':
        part = 2;
        break;
    case 'B':
        part = 3;
        break;
    } while (0);
    switch (W->row) {
    case 'B':
        row = 0;
        break;
    case 'O':
        row = 1;
        break;
    case 'D':
        row = 2;
        break;
    case 'J':
        row = 3;
        break;
    }
    if (chan == 'R') {
        v = D_80031A64[part][row][0];
        o->pos.vy = 185;
        o->pos.vz = 0;
    } else if (chan == 'G') {
        v = D_80031A64[part][row][1];
        o->pos.vy = 193;
        o->pos.vz = 0;
    } else {
        v = D_80031A64[part][row][2];
        o->pos.vy = 201;
        o->pos.vz = 0;
    }
    v += 224;
    do {    /* block boundary: keeps the vx store on a1 before the v0 copy */
        o->pos.vx = v;
        ((KnobWork *)&o->w)->owner = owner;
        ((KnobWork *)&o->w)->chan = chan;
        return o;
    } while (0);
}
