#include "ac1/ac1.h"
/* ovl201 0x8007E450 - update of the paint colour editor: up/down pick the channel R/G/B
 * (o+0x87), right/left raise/lower it (by 2 with L/R held, 0..64), on the shared group 'G'
 * the colour of group 1 is copied to all groups, triangle steps the colour slot
 * B/O/D/J (o+0x86) and its title, the two-digit values are redrawn; confirm keeps,
 * cancel restores the colours from the backup 0x801BCDE8; both go to 0x8007F598. */

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
    u8 r, g, b;
} Rgb3;

typedef struct {
    u8 pad7c[9];
    u8 part;                    /* +0x85 'H', 'C'/'G', 'A', 'B' */
    u8 slot;                    /* +0x86 'B', 'O', 'D', 'J' */
    u8 chan;                    /* +0x87 'R', 'G', 'B' */
} PaintEditWork;

extern s16 D_801A2318;
extern Rgb3 D_80031A64[4][4];
extern Rgb3 D_801BCDE8[4][4];
extern char *D_800B85BC;
extern char *D_800B85C0;
extern char *D_800B85C4;
extern char *D_800B85C8;
extern void *D_800B85CC[10];
extern void func_8005D15C(void *p, s32 x, s32 y);
extern void func_8005D2DC(void *p, s32 x, s32 y);
extern void func_80073DDC(void);
extern void func_8007F598(UiObj *o);

#define C D_80031A64

void func_8007E450(UiObj *o)
{
    PaintEditWork *w;
    u8 g;
    u8 s;
    u8 v;
    u8 t;
    s16 i;
    u32 b;

    do {    /* block boundary: picks the registers of g/s and the prologue order */
    w = (PaintEditWork *)&o->w;
    switch (w->part) {
    case 'H':
        g = 0;
        break;
    case 'C':
    case 'G':
        g = 1;
        break;
    case 'A':
        g = 2;
        break;
    case 'B':
        g = 3;
        break;
    }
    } while (0);
    switch (w->slot) {
    case 'B':
        s = 0;
        break;
    case 'O':
        s = 1;
        break;
    case 'D':
        s = 2;
        break;
    case 'J':
        s = 3;
        break;
    }
    if (D_801A2550.press & 0x1000) {
        D_801A2550.used |= 0x1000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 6 / D_801A2550.speed;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 6;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.delay2 = 12;
            }
        }
        switch (w->chan) {
        case 'R':
            w->chan = 'B';
            break;
        case 'G':
            w->chan = 'R';
            break;
        case 'B':
            w->chan = 'G';
            break;
        }
    } else if (D_801A2550.press & 0x4000) {
        D_801A2550.used |= 0x4000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 6 / D_801A2550.speed;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 6;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.delay2 = 12;
            }
        }
        switch (w->chan) {
        case 'R':
            w->chan = 'G';
            break;
        case 'G':
            w->chan = 'B';
            break;
        case 'B':
            w->chan = 'R';
            break;
        }
    }
    if (D_801A2550.press & 0x2000) {
        D_801A2550.used |= 0x2000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 0;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 0;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.delay2 = 12;
            }
        }
        switch (w->chan) {
        case 'R':
            if (D_801A2550.press & 0xC) {
                D_80031A64[g][s].r += 2;
            } else {
                D_80031A64[g][s].r += 1;
            }
            if (D_80031A64[g][s].r >= 65) {
                D_80031A64[g][s].r = 64;
            }
            break;
        case 'G':
            if (D_801A2550.press & 0x8) {
                D_80031A64[g][s].g += 2;
            } else {
                D_80031A64[g][s].g += 1;
            }
            if (D_80031A64[g][s].g >= 65) {
                D_80031A64[g][s].g = 64;
            }
            break;
        case 'B':
            if (D_801A2550.press & 0xC) {
                D_80031A64[g][s].b += 2;
            } else {
                D_80031A64[g][s].b += 1;
            }
            if (D_80031A64[g][s].b >= 65) {
                D_80031A64[g][s].b = 64;
            }
            break;
        }
    } else if (D_801A2550.press & 0x8000) {
        D_801A2550.used |= 0x8000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 0;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 0;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.delay2 = 12;
            }
        }
        switch (w->chan) {
        case 'R':
            if (D_801A2550.press & 0xC) {
                D_80031A64[g][s].r -= 2;
            } else {
                D_80031A64[g][s].r -= 1;
            }
            if (D_80031A64[g][s].r == 0xFE || D_80031A64[g][s].r == 0xFF) {
                D_80031A64[g][s].r = 0;
            }
            break;
        case 'G':
            if (D_801A2550.press & 0xC) {
                D_80031A64[g][s].g -= 2;
            } else {
                D_80031A64[g][s].g -= 1;
            }
            if (D_80031A64[g][s].g == 0xFE || D_80031A64[g][s].g == 0xFF) {
                D_80031A64[g][s].g = 0;
            }
            break;
        case 'B':
            if (D_801A2550.press & 0xC) {
                D_80031A64[g][s].b -= 2;
            } else {
                D_80031A64[g][s].b -= 1;
            }
            if (D_80031A64[g][s].b == 0xFE || D_80031A64[g][s].b == 0xFF) {
                D_80031A64[g][s].b = 0;
            }
            break;
        }
    }
    if (w->part == 'G') {
        for (i = 0; i < 4; i++) {
            if (i != 1) {
                C[i][0] = C[1][0];
                C[i][1] = C[1][1];
                C[i][2] = C[1][2];
                C[i][3] = C[1][3];
            }
        }
    }
    if ((D_801A2550.press & 0x10) && !(D_801A2550.held & 0x10)) {
        D_801A2550.used |= 0x10;
        switch (w->slot) {
        case 'B':
            w->slot = 'O';
            func_8005D15C(D_800B85C0, 960, 140);
            break;
        case 'O':
            w->slot = 'D';
            func_8005D15C(D_800B85C4, 960, 140);
            break;
        case 'D':
            w->slot = 'J';
            func_8005D15C(D_800B85C8, 960, 140);
            break;
        case 'J':
            w->slot = 'B';
            func_8005D15C(D_800B85BC, 960, 140);
            break;
        }
    }
    v = D_80031A64[g][s].r;
    t = v / 10;
    func_8005D2DC(D_800B85CC[t], 960, 132);
    func_8005D2DC(D_800B85CC[v - t * 10], 961, 132);
    v = D_80031A64[g][s].g;
    t = v / 10;
    func_8005D2DC(D_800B85CC[t], 962, 132);
    func_8005D2DC(D_800B85CC[v - t * 10], 963, 132);
    v = D_80031A64[g][s].b;
    t = v / 10;
    func_8005D2DC(D_800B85CC[t], 964, 132);
    func_8005D2DC(D_800B85CC[v - t * 10], 965, 132);
    func_80073DDC();
    b = D_801A2550.press & 0x60;
    if (b != 0) {
        D_801A2550.used |= b;
        if (D_801A2550.press & 0x20) {
            SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
            if (w->part == 'G') {
                for (i = 0; i < 4; i++) {
                    C[i][0] = D_801BCDE8[i][0];
                    C[i][1] = D_801BCDE8[i][1];
                    C[i][2] = D_801BCDE8[i][2];
                    C[i][3] = D_801BCDE8[i][3];
                }
            } else {
                C[g][0] = D_801BCDE8[g][0];
                C[g][1] = D_801BCDE8[g][1];
                C[g][2] = D_801BCDE8[g][2];
                C[g][3] = D_801BCDE8[g][3];
            }
            func_80073DDC();
        } else {
            SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        }
        o->update = func_8007F598;
    }
}
