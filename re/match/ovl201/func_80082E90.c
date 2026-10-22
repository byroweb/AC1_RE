#include "ac1/ac1.h"
/* ovl201 0x80082E90 - update of the kana character list (rows built by 0x800829F4, 18
 * chars per row, 8 rows visible): shows the scroll arrows, the d-pad moves the cursor
 * o+0x9C/o+0x9D inside the view (wrapping around the short last row), R1/L1 page by 8 rows
 * and redraw them, confirm writes the character into the AC or pilot name, confirm and
 * cancel close the list box (update 0x80083790). */

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

typedef struct {
    u8 pad7c[8];
    UiObj *arrow[4];            /* +0x84 up-left, +0x88 down-left, +0x8C up-right, +0x90 down-right */
    u8 pad94;
    u8 mode;                    /* +0x95 'A' = AC name, else pilot name */
    u8 len;                     /* +0x96 characters entered, 8 max */
    u8 rows;                    /* +0x97 last row index */
    u8 last_len;                /* +0x98 characters in the last row */
    u8 top;                     /* +0x99 first row shown */
    u8 pad9a[2];
    u8 cx;                      /* +0x9C cursor column 0..17 */
    u8 cy;                      /* +0x9D cursor row in the view 0..7 */
} KanaListWork;

extern char D_80031BD4[2][18];  /* AC name, pilot name: 2-byte chars, '>' mark */
extern char D_8017EF9C[8][38];
extern s16 D_801A2318;
extern void func_800506EC(s32 a);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 n);
extern UiObj *func_8005CB94(char *s, s32 a, s16 y, s32 b);
extern void func_80083894(u8 *x, u8 *y, s32 n);
extern void func_80083790(UiObj *o);

void func_80082E90(UiObj *o)
{
    KanaListWork *w = (KanaListWork *)&o->w;
    char *p;
    char *src;
    u16 i;
    u8 d;
    u8 d2;
    u8 d3;    /* one row-count local per branch: each gets its own register */

    if (w->top != 0) {
        w->arrow[0]->visible = 1;
        w->arrow[2]->visible = 1;
    } else {
        w->arrow[0]->visible = 0;
        w->arrow[2]->visible = 0;
    }
    if (w->rows >= w->top + 8) {
        w->arrow[1]->visible = 1;
        w->arrow[3]->visible = 1;
    } else {
        w->arrow[1]->visible = 0;
        w->arrow[3]->visible = 0;
    }
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
        if (w->top + 7 < w->rows) {
            if (w->cx == 0) {
                w->cx = 17;
            } else {
                w->cx--;
            }
        } else {
            d = w->rows - w->top;
            if (w->cx == 0) {
                if (w->cy == d) {
                    w->cx = w->last_len - 1;
                } else {
                    w->cx = 17;
                }
            } else {
                w->cx--;
            }
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
        if (w->top + 7 < w->rows) {
            if (w->cx == 17) {
                w->cx = 0;
            } else {
                w->cx++;
            }
        } else if ((w->cy == (u8)(w->rows - w->top) && w->cx == w->last_len - 1) || w->cx == 17) {
            w->cx = 0;
        } else {
            w->cx++;
        }
    }
    if (D_801A2550.press & 0x1000) {
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
        if (w->top + 7 < w->rows) {
            if (w->cy == 0) {
                w->cy = 7;
            } else {
                w->cy--;
            }
        } else if (w->rows < w->top + 7) {
            d2 = w->rows - w->top;
            if (w->cy == 0) {
                if (w->cx >= w->last_len) {
                    w->cy = d2 - 1;
                } else {
                    w->cy = d2;
                }
            } else {
                w->cy--;
            }
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
        if (w->top + 7 < w->rows) {
            if (w->cy == 7) {
                w->cy = 0;
            } else {
                w->cy++;
            }
        } else if (w->rows < w->top + 7) {
            if (w->cy == (u8)(w->rows - w->top)
                || (w->cy == (u8)(w->rows - w->top) - 1 && w->cx >= w->last_len)) {
                w->cy = 0;
            } else {
                w->cy++;
            }
        }
    } else if (D_801A2550.press & 0x8) {
        D_801A2550.used |= 0x8;
        if (w->rows >= w->top + 8) {
            w->top += 8;
            func_800506EC(8);
            if (w->top + 8 >= w->rows) {
                d3 = w->rows - w->top;
                if (d3 < w->cy) {
                    w->cy = d3;
                }
                if (w->cy == d3) {
                    if (w->cx >= w->last_len - 1) {
                        w->cx = w->last_len - 1;
                    }
                }
            }
            func_80083894(&w->cx, &w->cy, 8);
            for (i = w->top; i <= w->rows; i++) {
                if (i == w->top + 8) {
                    break;
                }
                func_8005CB94(D_8017EF9C[i], 8, (i - w->top) * 16 + 4, 8)->unk14 = 3;
            }
        }
    } else if (D_801A2550.press & 0x4) {
        D_801A2550.used |= 0x4;
        if (w->top != 0) {
            w->top -= 8;
            func_800506EC(8);
            func_80083894(&w->cx, &w->cy, 8);
            for (i = w->top; i <= w->rows; i++) {
                if (i == w->top + 8) {
                    break;
                }
                func_8005CB94(D_8017EF9C[i], 8, (i - w->top) * 16 + 4, 8)->unk14 = 3;
            }
        }
    }
    if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        w->arrow[0]->kill = 1;
        w->arrow[2]->kill = 1;
        w->arrow[1]->kill = 1;
        w->arrow[3]->kill = 1;
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
        src = D_8017EF9C[(u8)(w->top + w->cy)];
        *p++ = src[w->cx * 2];
        *p++ = src[w->cx * 2 + 1];
        *p = '>';
        if (w->len != 8) {
            w->len++;
        }
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(8, 40, 304, 136, 7);
        o->update = func_80083790;
    } else if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        w->arrow[0]->kill = 1;
        w->arrow[2]->kill = 1;
        w->arrow[1]->kill = 1;
        w->arrow[3]->kill = 1;
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(8, 40, 304, 136, 7);
        o->update = func_80083790;
    }
}
