#include "ac1/ac1.h"
/* ovl201 0x8007FB44 - make the colour-value marker of the paint editor: sprite 0x800B84E4
 * placed at x = 226 + value of the edited component (owner chan 'R'/'G'/else B selects the
 * row 189/197/205); update 0x8007FD48 */
typedef struct {
    u8 pad7c[9];
    u8 part;                    /* +0x85 */
    u8 row;                     /* +0x86 */
    u8 chan;                    /* +0x87 */
} PaintWork;

extern u8 D_80031A64[4][4][3];
#define W ((PaintWork *)&owner->w)
extern u8 D_800B84E4[];
extern void func_8007FD48(UiObj *o);

UiObj *func_8007FB44(UiObj *owner)
{
    UiObj *o;

    u8 part;
    u8 row;
    s32 v;

    o = ui_obj_alloc(2);
    o->ctor = func_8007FB44;
    o->update = func_8007FD48;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 21;
    o->unk14 = 1;
    o->res = D_800B84E4;
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
    if (W->chan == 'R') {
        v = D_80031A64[part][row][0];
        o->pos.vy = 189;
        o->pos.vz = 0;
    } else if (W->chan == 'G') {
        v = D_80031A64[part][row][1];
        o->pos.vy = 197;
        o->pos.vz = 0;
    } else {
        v = D_80031A64[part][row][2];
        o->pos.vy = 205;
        o->pos.vz = 0;
    }
    v += 226;
    o->pos.vx = v;
    o->w.list.src = owner;
    do {    /* block boundary: keeps the stores on a1 before the v0 copy */
        return o;
    } while (0);
}
