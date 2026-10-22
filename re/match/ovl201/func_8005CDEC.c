#include "ac1/ac1.h"

extern MATRIX D_800B8F24;
extern MATRIX D_800B8F84;
extern MATRIX D_800B8FA4;
extern MATRIX D_800B9024;
extern s32 D_801A2574;
extern void func_8008729C(UiObj *);

/* Sweep the light-colour matrices up and down (rank = direction +1/-1). */
void func_8005CDEC(UiObj *o)
{
    s32 step;
    s16 *dir;

    dir = &o->w.panel.rank;
    step = 16;
    if (D_801A2574 == 0) {
        step = 8;
    }
    if (*dir == 1) {
        D_800B8F24.m[0][2] += step;
        D_800B8F24.m[1][2] += step;
        D_800B8F24.m[2][2] += step;
        if (D_800B8F24.m[0][2] >= 384) {
            *dir = -1;
        }
    } else {
        D_800B8F24.m[0][2] -= step;
        D_800B8F24.m[1][2] -= step;
        D_800B8F24.m[2][2] -= step;
        if (D_800B8F24.m[0][2] <= 128) {
            *dir = 1;
        }
    }
    if (*dir == 1) {
        D_800B8F84.m[1][2] += step;
    } else {
        D_800B8F84.m[1][2] -= step;
    }
    if (obj_find_by_handle(func_8008729C) != &g_ui_none) {
        if (*dir == 1) {
            D_800B8FA4.m[0][0] += step;
            D_800B8FA4.m[0][1] += step;
            D_800B8FA4.m[0][2] += step;
            D_800B8FA4.m[1][0] += step;
            D_800B8FA4.m[1][1] += step;
            D_800B8FA4.m[1][2] += step;
            D_800B8FA4.m[2][0] += step;
            D_800B8FA4.m[2][1] += step;
            D_800B8FA4.m[2][2] += step;
        } else {
            D_800B8FA4.m[0][0] -= step;
            D_800B8FA4.m[0][1] -= step;
            D_800B8FA4.m[0][2] -= step;
            D_800B8FA4.m[1][0] -= step;
            D_800B8FA4.m[1][1] -= step;
            D_800B8FA4.m[1][2] -= step;
            D_800B8FA4.m[2][0] -= step;
            D_800B8FA4.m[2][1] -= step;
            D_800B8FA4.m[2][2] -= step;
        }
    } else {
        if (*dir == 1) {
            D_800B8FA4.m[0][2] += step;
        } else {
            D_800B8FA4.m[0][2] -= step;
        }
    }
    if (*dir == 1) {
        D_800B9024.m[0][2] += step;
        D_800B9024.m[1][2] += step;
    } else {
        D_800B9024.m[0][2] -= step;
        D_800B9024.m[1][2] -= step;
    }
}
