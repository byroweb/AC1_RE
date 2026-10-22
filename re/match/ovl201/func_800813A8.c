#include "ac1/ac1.h"
/* ovl201 0x800813A8 - update of the kana name-entry grid (11 x 5 keys; column 10 holds
 * two extra marks, the kana list, SPACE and END): the d-pad moves the cursor with jumps
 * around the holes, confirm on a kana key opens its character list (box, update
 * 0x80082820), on a mark / SPACE writes it into the AC or pilot name, on END asks for
 * confirmation (update 0x800824D0); cancel erases, START jumps to END. */

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
extern u8 *D_800B85F4[5];       /* key grid rows, 2 bytes per column */
extern char D_8004C878[];
extern void func_8005D8D4(void *msg, u8 *out, s32 a, s32 b);
extern void func_800506EC(s32 a);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 n);
extern void func_800824D0(UiObj *o);
extern void func_80082820(UiObj *o);

void func_800813A8(UiObj *o)
{
    NameWork *w = (NameWork *)&o->w;
    char *p;
    s32 r;
    u8 col;
    u8 row;

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
            switch (w->k.c.row) {
            case 0:
            case 1:
            case 3:
            case 4:
                w->k.c.col = 10;
                break;
            case 2:
                w->k.c.col = 9;
                break;
            }
        } else if (w->k.c.col == 8) {
            r = w->k.c.row;
            if (r < 5) {
                if (r >= 3) {
                    w->k.c.col = 6;
                } else {
                    w->k.c.col = 7;
                }
            } else {
                w->k.c.col = 7;
            }
        } else if (w->k.c.col == 10) {
            if (w->k.c.row == 3 || w->k.c.row == 4) {
                w->k.c.col = 8;
            } else {
                w->k.c.col = 9;
            }
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
        if (w->k.c.col == 8 && (w->k.c.row == 3 || w->k.c.row == 4)) {
            w->k.c.col = 10;
        } else if (w->k.c.col == 10) {
            w->k.c.col = 0;
        } else if (w->k.c.col == 6) {
            r = w->k.c.row;
            if (r < 5) {
                if (r >= 3) {
                    w->k.c.col = 8;
                } else {
                    w->k.c.col = 7;
                }
            } else {
                w->k.c.col = 7;
            }
        } else if (w->k.c.col == 8) {
            r = w->k.c.row;
            if (r < 5) {
                if (r >= 3) {
                    w->k.c.col = 0;
                } else {
                    w->k.c.col = 9;
                }
            } else {
                w->k.c.col = 9;
            }
        } else if (w->k.c.col == 9) {
            if (w->k.c.row < 2) {
                w->k.c.col = 10;
            } else {
                w->k.c.col = 0;
            }
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
        if (w->k.c.col == 10) {
            if (w->k.c.row == 0) {
                w->k.c.row = 4;
            } else if (w->k.c.row == 3) {
                w->k.c.row = 1;
            } else {
                w->k.c.row--;
            }
        } else if (w->k.c.row == 0) {
            if (w->k.c.col == 7 || w->k.c.col == 9) {
                w->k.c.row = 2;
            } else {
                w->k.c.row = 4;
            }
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
        col = w->k.c.col;
        if (col == 10) {
            row = w->k.c.row;
            if (row == 1) {
                w->k.c.row = 3;
            } else if (row == 4) {
                w->k.c.row = 0;
            } else {
                w->k.c.row = row + 1;
            }
        } else {
            row = w->k.c.row;
            if (row == 2 && (col == 7 || col == 9)) {
                w->k.c.row = 0;
            } else {
                w->k.c.row = row + 1;
            }
        }
        if (w->k.c.row == 5) {
            w->k.c.row = 0;
        }
    }
    if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        if (w->k.c.col == 10) {
            switch (w->k.c.row) {
            case 0:
            case 1:
            case 3:
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
                if (w->k.c.row != 3) {
                    *p++ = D_800B85F4[w->k.c.row][w->k.c.col * 2];
                    *p++ = D_800B85F4[w->k.c.row][w->k.c.col * 2 + 1];
                } else {
                    *p++ = 0x81;    /* full-width space */
                    *p++ = 0x40;
                }
                *p = '>';
                if (w->len != 8) {
                    w->len++;
                }
                return;
            case 4:
                func_8005D8D4(D_8004C878, &w->answer, 12, 0);
                o->update = func_800824D0;
                return;
            }
        }
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(24, 64, 192, 88, 3);
        o->update = func_80082820;
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
        w->k.c.col = 10;
        w->k.c.row = 4;
    }
}
