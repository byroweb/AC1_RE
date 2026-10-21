#include "ac1/ac1.h"

typedef struct {
    u8 pad7c[0x08];
    u16 timer;                  /* +0x84 */
    u8 pad86[6];
    u16 id;                     /* +0x8C */
    u8 pad8e[2];
    UiObj *child;               /* +0x90 */
} Work21;

extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern s32 D_801A2550;
extern s32 D_801A2568;
extern s32 D_801A2574;
extern void func_8005E1E8(UiObj *);

void func_8005E040(UiObj *o) {
    Work21 *w = (Work21 *)&o->w;
    s32 lim;
    s32 f;

    /* case 3 shares the default body; dropping it moves the switch pivot */
    switch (D_801A2574) {
    case 0:
        lim = 120;
        break;
    case 2:
        lim = 60;
        break;
    case 3:
    default:
        lim = 30;
        break;
    }
    if (lim < (s16)++w->timer || (D_801A2568 & 0x600060)) {
        f = D_801A2568;
        if (f & 0x40) {
            D_801A2550 |= 0x40;
        }
        if (f & 0x20) {
            D_801A2550 |= 0x20;
        }
        if (f & 0x400000) {
            D_801A2550 |= 0x400000;
        }
        if (f & 0x200000) {
            D_801A2550 |= 0x200000;
        }
        w->child->kill = 1;
        obj_find_by_id((s16)(w->id - 1))->kill = 1;
        func_8005C2D0(0x14, 0x68, 0x118, 0x20, (s16)(w->id - 1));
        o->update = func_8005E1E8;
    }
}
