#include "ac1/ac1.h"
/* ovl201 0x80084474 - build the visible rows (max 5) of the option list from entry pos:
 * one row per available option bit (D_800485F0), a check mark when it is fitted (D_80039CA0) */
typedef struct UiPairWork {
    u8 pad7c[0x14];
    UiObj *a[6];                /* +0x90 check marks */
    UiObj *b[5];                /* +0xA8 name labels */
    u8 padBC[0xA];
    u8 pos;                     /* +0xC6 first entry shown */
} UiPairWork;
extern u32 D_800485F0;
extern u16 D_80039CA0;
extern u8 D_800B8568[];
extern u8 D_800BB11C[][0x2C];
extern UiObj *func_8005D754(s32 layer, void *frame, s32 x, s32 y, s32 kind);
extern UiObj *func_8005D80C(s32 layer, void *text, s32 x, s32 w, s32 y, s32 kind);

void func_80084474(UiObj *o)
{
    UiPairWork *w;
    UiObj *t;
    u16 fit;
    u32 avail;
    s16 y;
    s16 i;
    s16 j;
    s16 light;
    u8 k;

    y = 0;
    i = 0;
    k = ((UiPairWork *)&o->w)->pos;
    avail = D_800485F0;
    fit = D_80039CA0;
    avail >>= k;
    fit >>= k;
    w = (UiPairWork *)&o->w;
    for (j = k; j < 11; j++) {
        if (i >= 5) {
            break;
        }
        if (avail & 1) {
            light = 21;
            if (fit & 1) {
                w->a[i] = func_8005D754(2, D_800B8568, 0, y, 4);
            } else {
                light = 16;
            }
            t = func_8005D80C(2, D_800BB11C[k], 16, 36, (s16)(y + 4), 4);
            y += 24;
            w->b[i] = t;
            i++;
            t->light = light;
        }
        k++;
        fit >>= 1;
        avail >>= 1;
    }
}
