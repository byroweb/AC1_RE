#include "ac1/ac1.h"
/* ovl201 0x8007DFF0 - colour editor setup: once the box is gone, draw the frames, make
 * the R/G/B sliders, the part-group title and selectors, copy the AC colour table into
 * the edit buffer and add the swatch; update := 0x8007E450 */
typedef struct {
    s8 r, g, b;
} Rgb3;

typedef struct PaintEditWork {
    u8 pad7c[9];
    u8 part;                    /* +0x85 'G','H','C','A','B' */
    u8 slot;                    /* +0x86 */
    u8 chan;                    /* +0x87 */
    u8 pad88[4];
    UiObj *frame;               /* +0x8C */
    UiObj *mark[3];             /* +0x90 */
    UiObj *slider[3];           /* +0x9C R, G, B */
    UiObj *list;                /* +0xA8 */
    UiObj *swatch;              /* +0xAC */
} PaintEditWork;

extern u8 D_80031A5B;
extern u8 D_80031A63;
extern Rgb3 D_80031A64[4][4];
extern Rgb3 D_801BCDE8[4][4];
extern u8 D_8004C594[];
extern u8 D_8004C59C[];
extern char *D_800B824C;
extern char *D_800B8250;
extern char *D_800B8254;
extern char *D_800B8258;
extern char *D_800B85BC;
extern u8 D_800B83DC[];
extern u8 D_800B83E8[];
extern u8 D_800B8484[];
extern u8 D_800B8490[];
extern u8 D_800B849C[];
extern u8 D_800B84A8[];
extern u8 D_800B84B4[];
extern u8 D_800B84C0[];
extern u8 D_800B84CC[];
extern void func_8005BA2C();
extern UiObj *func_8005D754(s32 layer, void *frame, s32 x, s32 y, s32 kind);
extern void func_8005D15C(void *p, s32 x, s32 y);
extern void func_8005D2DC(void *p, s32 x, s32 y);
extern void func_8005DE08(s32 a, s32 b, s32 c);
extern UiObj *func_8007F7B0(s32 chan, UiObj *o);
extern UiObj *func_8007FB44(UiObj *o);
extern UiObj *func_8007F2A8(UiObj *o);
extern void func_800800A4(s32 a, s32 b, s32 c);
extern UiObj *func_80080224(s32 kind, UiObj *parent, s32 n);
extern void func_8007E450(UiObj *o);

void func_8007DFF0(UiObj *o)
{
    PaintEditWork *w;
    UiObj *t;
    char *title;
    s16 i;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        w = (PaintEditWork *)&o->w;
        func_8005D754(2, D_800B8484, 0, 8, 4);
        func_8005D754(1, D_800B83DC, 0, 0, 4);
        func_8005D754(1, D_800B83E8, 136, 0, 4);
        w->frame = func_8005D754(3, D_800B8490, 160, 177, 2);
        func_8005D2DC(D_8004C594, 960, 132);
        w->mark[0] = func_8005D754(1, D_800B849C, 296, 185, 2);
        w->mark[1] = func_8005D754(1, D_800B84A8, 296, 193, 2);
        w->mark[2] = func_8005D754(1, D_800B84B4, 296, 201, 2);
        w->slider[0] = func_8007F7B0('R', o);
        w->slider[1] = func_8007F7B0('G', o);
        w->slider[2] = func_8007F7B0('B', o);
        w->list = func_8007FB44(o);
        w->slot = 'B';
        w->chan = 'R';
        func_8005DE08(960, 156, 15);
        func_8005D15C(D_800B85BC, 960, 140);
        func_8005D754(2, D_800B84C0, 4, 4, 8);
        func_8005D754(1, D_800B84CC, 0, 0, 8);
        switch (w->part) {
        case 'G':
            func_8005D15C(D_8004C59C, 962, 116);
            t = func_80080224(6, &g_ui_none, 0);
            func_80080224(4, t, 0);
            func_80080224(3, t, 0);
            if (D_80031A63 < 21 || D_80031A63 == 255 || D_80031A5B >= 8) {
                func_80080224(5, t, 0);
                func_80080224(5, t, 1);
            } else {
                func_80080224(5, t, 2);
                func_80080224(5, t, 3);
            }
            break;
        case 'H':
            func_800800A4(3, 2, 4);
            func_8005D15C(D_800B824C, 962, 116);
            break;
        case 'C':
            func_800800A4(4, 0, 0);
            func_8005D15C(D_800B8250, 962, 116);
            break;
        case 'A':
            func_800800A4(5, 3, 2);
            func_8005D15C(D_800B8254, 962, 116);
            break;
        case 'B':
            func_800800A4(6, 11, 1);
            func_8005D15C(D_800B8258, 962, 116);
            break;
        }
        for (i = 0; i < 4; i++) {
            D_801BCDE8[i][0] = D_80031A64[i][0];
            D_801BCDE8[i][1] = D_80031A64[i][1];
            D_801BCDE8[i][2] = D_80031A64[i][2];
            D_801BCDE8[i][3] = D_80031A64[i][3];
        }
        w->swatch = func_8007F2A8(o);
        o->update = func_8007E450;
    }
}
