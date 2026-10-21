#include "ac1/ac1.h"
/* ovl201 0x8004ECAC - create the bar image for the slide mode, set where the
 * slide stops and start sliding (func_8004EDC0). */

typedef struct {
    u8 raw[0xC];
} Entry0C;

extern Entry0C D_800B6E4C[];
extern u8 D_800B91A8;
extern void func_80073D20(void);
extern UiObj *func_8005CAE0(Entry0C *e, s32 x, s16 y, s32 kind);
extern void func_8004EDC0(UiObj *o);

void func_8004ECAC(UiObj *o)
{
    UiWork *w = &o->w;
    Entry0C *e;
    s16 y;

    switch (o->w.slide.mode) {
    case 0:
        if (D_800B91A8) {
            D_800B91A8 = 0;
        }
        func_80073D20();
        e = &D_800B6E4C[0];
        w->slide.stop = 32;
        y = 16;
        break;
    case 1:
        e = &D_800B6E4C[1];
        w->slide.stop = 8;
        y = 48;
        break;
    case 2:
        e = &D_800B6E4C[2];
        w->slide.stop = 8;
        y = 48;
        break;
    case 3:
        e = &D_800B6E4C[3];
        w->slide.stop = 16;
        y = 16;
        break;
    case 4:
        e = &D_800B6E4C[4];
        w->slide.stop = 16;
        y = 48;
        break;
    case 5:
        e = &D_800B6E4C[5];
        w->slide.stop = 16;
        y = 16;
        break;
    }
    w->slide.bar = func_8005CAE0(e, 448, y, 2);
    o->update = func_8004EDC0;
}
