#include "ac1/ac1.h"
/* ovl201 0x8004E894 - turn the ring one step (16 steps per turn): spin every
 * panel by the D_800B6EAC step table, raise the front panel's label and lower
 * the others, and fade matrix 3 from matrix 2 towards matrix 0 and matrix 4 back towards
 * matrix 2 (first two columns), 1/16 of the difference a step. */

extern u16 D_800B6EAC[];
extern MATRIX D_800B8E44[5];
extern void func_8004E2BC(UiObj *o);
extern void func_8004F024(UiObj *o);

void func_8004E894(UiObj *o)
{
    s16 i;
    UiObj *label;

    for (i = 0; i < 6; i++) {
        if (o->w.ring.dir > 0) {
            o->w.ring.item[i]->rot.vy += D_800B6EAC[o->w.ring.step];
        } else {
            o->w.ring.item[i]->rot.vy -= D_800B6EAC[o->w.ring.step];
        }
        if (i == o->w.ring.front) {
            o->w.ring.item[i]->w.panel.label->rot.vx += 55;
        } else {
            label = o->w.ring.item[i]->w.panel.label;
            if (label->rot.vx != -1136) {
                label->rot.vx -= 55;
            }
        }
    }
    D_800B8E44[3].m[0][0] += (u32)(D_800B8E44[0].m[0][0] - D_800B8E44[2].m[0][0]) >> 4;
    D_800B8E44[3].m[0][1] += (u32)(D_800B8E44[0].m[0][1] - D_800B8E44[2].m[0][1]) >> 4;
    D_800B8E44[3].m[1][0] += (u32)(D_800B8E44[0].m[1][0] - D_800B8E44[2].m[1][0]) >> 4;
    D_800B8E44[3].m[1][1] += (u32)(D_800B8E44[0].m[1][1] - D_800B8E44[2].m[1][1]) >> 4;
    D_800B8E44[3].m[2][0] += (u32)(D_800B8E44[0].m[2][0] - D_800B8E44[2].m[2][0]) >> 4;
    D_800B8E44[3].m[2][1] += (u32)(D_800B8E44[0].m[2][1] - D_800B8E44[2].m[2][1]) >> 4;
    D_800B8E44[4].m[0][0] -= (u32)(D_800B8E44[0].m[0][0] - D_800B8E44[2].m[0][0]) >> 4;
    D_800B8E44[4].m[0][1] -= (u32)(D_800B8E44[0].m[0][1] - D_800B8E44[2].m[0][1]) >> 4;
    D_800B8E44[4].m[1][0] -= (u32)(D_800B8E44[0].m[1][0] - D_800B8E44[2].m[1][0]) >> 4;
    D_800B8E44[4].m[1][1] -= (u32)(D_800B8E44[0].m[1][1] - D_800B8E44[2].m[1][1]) >> 4;
    D_800B8E44[4].m[2][0] -= (u32)(D_800B8E44[0].m[2][0] - D_800B8E44[2].m[2][0]) >> 4;
    D_800B8E44[4].m[2][1] -= (u32)(D_800B8E44[0].m[2][1] - D_800B8E44[2].m[2][1]) >> 4;
    if (D_800B8E44[0].m[0][0] < (s16)D_800B8E44[3].m[0][0]) {
        D_800B8E44[3].m[0][0] = D_800B8E44[0].m[0][0];
        D_800B8E44[3].m[0][1] = D_800B8E44[0].m[0][1];
        D_800B8E44[3].m[1][0] = D_800B8E44[0].m[1][0];
        D_800B8E44[3].m[1][1] = D_800B8E44[0].m[1][1];
        D_800B8E44[3].m[2][0] = D_800B8E44[0].m[2][0];
        D_800B8E44[3].m[2][1] = D_800B8E44[0].m[2][1];
    }
    if ((s16)D_800B8E44[4].m[0][0] < D_800B8E44[2].m[0][0]) {
        D_800B8E44[4].m[0][0] = D_800B8E44[2].m[0][0];
        D_800B8E44[4].m[0][1] = D_800B8E44[2].m[0][1];
        D_800B8E44[4].m[1][0] = D_800B8E44[2].m[1][0];
        D_800B8E44[4].m[1][1] = D_800B8E44[2].m[1][1];
        D_800B8E44[4].m[2][0] = D_800B8E44[2].m[2][0];
        D_800B8E44[4].m[2][1] = D_800B8E44[2].m[2][1];
    }
    if (++o->w.ring.step == 16) {
        func_8004F024(o);
        o->update = func_8004E2BC;
    }
}
