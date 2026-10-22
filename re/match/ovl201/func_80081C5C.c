#include "ac1/ac1.h"
/* ovl201 0x80081C5C - update of the name-entry object (ctor 0x800810F0):
 * the d-pad moves the cursor over the 17 x 8 character grid (the grid has
 * holes, so some moves jump), confirm writes the character under the cursor
 * (or a full-width space) into the AC / pilot name and moves the '>' mark,
 * cancel erases the last character, START jumps to END; confirm on END asks
 * "Confirm the registered name?" and hands over to 0x8008240C. */

/* pad block at 0x801A2550 */
typedef struct {
    u32 used;           /* 0x801A2550 buttons consumed this frame */
    s16 delay1;         /* 0x801A2554 first repeat delay */
    s16 delay2;         /* 0x801A2556 */
    u8 pad08[0x10];
    u32 press;          /* 0x801A2568 */
    u32 held;           /* 0x801A256C */
    u8 pad20[4];
    s32 speed;          /* 0x801A2574 */
} PadState;

extern PadState D_801A2550;

/* name-entry work area (UiObj +0x7C) */
typedef struct {
    void *dst;          /* +0x7C */
    void *arg;          /* +0x80 */
    u8 pad84[0x10];
    u8 answer;          /* +0x94 yes/no result of the confirm box */
    u8 mode;            /* +0x95 'A' = AC name, else pilot name */
    u8 len;             /* +0x96 characters entered, 8 max */
    u8 pad97;
    union {
        struct {
            u8 pad98[2];
            u8 col;     /* +0x9A 0..16 */
            u8 row;     /* +0x9B 0..7 */
        } c;
        u32 word;       /* (word & 0xFFFF0000) >> 16 = row << 8 | col */
    } k;
} NameWork;

extern char D_80031BD4[2][18];  /* AC name, pilot name: 2-byte chars, '>' mark */
extern s16 D_801A2318;
extern u8 *D_800B8608[8];       /* character grid rows, 2 bytes per column */
extern char D_8004C878[];
extern void func_8005D8D4(void *msg, u8 *out, s32 a, s32 b);
extern void func_8008240C(UiObj *o);

void func_80081C5C(UiObj *o)
{
    NameWork *w = (NameWork *)&o->w;
    char *p;
    u8 r;

    if (D_801A2550.press & 0x8000) {
        D_801A2550.used |= 0x8000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 4 / D_801A2550.speed;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 4;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.delay2 = 12;
            }
        }
        if (w->k.c.col == 0) {
            if (w->k.c.row == 1 || w->k.c.row == 3) {
                w->k.c.col = 8;
            } else {
                w->k.c.col = 16;
            }
        } else if (w->k.c.row >= 4 && w->k.c.row < 6 && w->k.c.col == 12) {
            w->k.c.col = 9;
        } else if ((w->k.word & 0xFFFF0000) == 0x070F0000) {
            w->k.c.col = 6;
        } else {
            w->k.c.col--;
        }
    } else if (D_801A2550.press & 0x2000) {
        D_801A2550.used |= 0x2000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 4 / D_801A2550.speed;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 4;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.delay2 = 12;
            }
        }
        if (w->k.c.col == 16 || ((w->k.c.row == 1 || w->k.c.row == 3) && w->k.c.col == 8)) {
            w->k.c.col = 0;
        } else if (w->k.c.row >= 4 && w->k.c.row < 6 && w->k.c.col == 9) {
            w->k.c.col = 12;
        } else if ((w->k.word & 0xFFFF0000) == 0x07060000) {
            w->k.c.col = 15;
        } else {
            w->k.c.col++;
        }
    } else if (D_801A2550.press & 0x1000) {
        D_801A2550.used |= 0x1000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 4 / D_801A2550.speed;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 4;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.delay2 = 12;
            }
        }
        r = w->k.c.row;
        if (r == 0) {
            if (w->k.c.col < 7 || (w->k.c.col >= 15 && w->k.c.col < 17)) {
                w->k.c.row = 7;
            } else {
                w->k.c.row = 6;
            }
        } else if ((r == 1 || r == 3) && w->k.c.col >= 9) {
            w->k.c.row = r - 2;
        } else if (w->k.c.row == 4 && (w->k.c.col >= 12 || w->k.c.col == 9)) {
            w->k.c.row = 2;
        } else if (w->k.c.row == 6 && w->k.c.col >= 10 && w->k.c.col < 12) {
            w->k.c.row = 2;
        } else if (w->k.c.row == 2 && w->k.c.col >= 9) {
            w->k.c.row = 0;
        } else {
            w->k.c.row--;
        }
    } else if (D_801A2550.press & 0x4000) {
        D_801A2550.used |= 0x4000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 4 / D_801A2550.speed;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 4;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.delay2 = 12;
            }
        }
        r = w->k.c.row;
        if (r == 7) {
            w->k.c.row = 0;
        } else if (r == 6) {
            if (w->k.c.col < 7 || (w->k.c.col >= 15 && w->k.c.col < 17)) {
                w->k.c.row = 7;
            } else {
                w->k.c.row = 0;
            }
        } else if (r == 2) {
            if (w->k.c.col == 9 || w->k.c.col >= 12) {
                w->k.c.row = 4;
            } else if (w->k.c.col < 9) {
                w->k.c.row = 3;
            } else {
                w->k.c.row = 6;
            }
        } else if (r == 0) {
            if (w->k.c.col < 9) {
                w->k.c.row = 1;
            } else {
                w->k.c.row = 2;
            }
        } else {
            w->k.c.row = r + 1;
        }
    }
    if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        if ((w->k.word & 0xFFFF0000) == 0x07100000) {
            func_8005D8D4(D_8004C878, &w->answer, 12, 0);
            o->update = func_8008240C;
        } else {
            if (w->mode == 'A') {
                if (w->len != 8) {
                    p = &D_80031BD4[0][w->len * 2];
                } else {
                    p = &D_80031BD4[0][14];
                }
            } else {
                if (w->len != 8) {
                    p = &D_80031BD4[1][w->len * 2];
                } else {
                    p = &D_80031BD4[1][14];
                }
            }
            if ((w->k.word & 0xFFFF0000) == 0x070F0000) {
                *p++ = 0x81;    /* full-width space */
                *p++ = 0x40;
            } else {
                *p++ = D_800B8608[w->k.c.row][w->k.c.col * 2];
                *p++ = D_800B8608[w->k.c.row][w->k.c.col * 2 + 1];
            }
            *p = '>';
            if (w->len != 8) {
                w->len++;
            }
        }
    } else if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        p = D_80031BD4[1];
        if (w->mode == 'A') {
            p = D_80031BD4[0];
        }
        if (w->len != 0) {
            p[--w->len * 2 + 1] = 0;
            p[w->len * 2 + 2] = 0;
            p[w->len * 2] = '>';
        } else {
            SsUtKeyOn(D_801A2318, 0, 7, 60, 0, 127, 127);
        }
    } else if (D_801A2550.press & 0x800) {
        D_801A2550.used |= 0x800;
        w->k.c.col = 16;
        w->k.c.row = 7;
    }
}
