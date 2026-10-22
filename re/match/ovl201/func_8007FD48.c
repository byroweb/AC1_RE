#include "ac1/ac1.h"
/* ovl201 0x8007FD48 - place the owner's channel marker: like func_8007F9C4
 * but the channel letter is the owner's (+0x87), on rows 189 / 197 / 205
 * two pixels further right. */

typedef struct {
    UiObj *owner;               /* +0x7C */
    u8 chan;                    /* +0x80 'R', 'G' or 'B' */
} KnobWork;

typedef struct {
    u8 pad7c[9];
    u8 part;                    /* +0x85 */
    u8 row;                     /* +0x86 */
    u8 chan;                    /* +0x87 */
} PaintWork;

extern u8 D_80031A64[4][4][3];

/* part and row are parameter slots in the original (retail keeps them in a1 / a3). */
void func_8007FD48(UiObj *o, u8 part, s32 unused, u8 row)
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
    if (((PaintWork *)&w->owner->w)->chan == 'R') {
        v = D_80031A64[part][row][0];
        o->pos.vy = 189;
        o->pos.vz = 0;
    } else if (((PaintWork *)&w->owner->w)->chan == 'G') {
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
}
