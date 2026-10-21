#include "ac1/ac1.h"
/* ovl201 0x8007F9C4 - place a paint slider knob: the owner's part letter
 * (+0x85) and colour-row letter (+0x86) pick the paint colour, the slider's
 * channel letter ('R', 'G', else B) the component and the row on screen. */

typedef struct {
    UiObj *owner;               /* +0x7C */
    u8 chan;                    /* +0x80 'R', 'G' or 'B' */
} KnobWork;

typedef struct {
    u8 pad7c[9];
    u8 part;                    /* +0x85 */
    u8 row;                     /* +0x86 */
} PaintWork;

extern u8 D_80031A64[4][4][3];

/* part and row are parameter slots in the original (retail keeps them in a1 / a3). */
void func_8007F9C4(UiObj *o, u8 part, s32 unused, u8 row)
{
    KnobWork *w = (KnobWork *)&o->w;
    s32 v;
    s32 y;

    switch (((PaintWork *)&w->owner->w)->part) {
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
    }
    switch (((PaintWork *)&w->owner->w)->row) {
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
    if (w->chan == 'R') {
        v = D_80031A64[part][row][0];
        o->pos.vy = 185;
        o->pos.vz = 0;
    } else if (w->chan == 'G') {
        v = D_80031A64[part][row][1];
        o->pos.vy = 193;
        o->pos.vz = 0;
    } else {
        v = D_80031A64[part][row][2];
        o->pos.vy = 201;
        o->pos.vz = 0;
    }
    v += 224;
    o->pos.vx = v;
}
