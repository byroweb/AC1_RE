#include "ac1/ac1.h"

extern void func_8005CDEC(UiObj *);
extern MATRIX D_800B8F24;
extern MATRIX D_800B8F84;
extern MATRIX D_800B8FA4;
extern MATRIX D_800B9024;

UiObj *func_8005CC48(void)
{
    UiObj *o;

    if (obj_find_by_handle(func_8005CC48) != &g_ui_none) {
        return &g_ui_none;
    }
    {
        o = ui_obj_alloc(0);
        o->update = func_8005CDEC;
        o->destroy = ui_obj_nop;
        o->w.panel.rank = 1;
        o->ctor = func_8005CC48;
        o->visible = 0;
        o->attached = 0;
        o->parent = 0;
        o->has_xform = 0;
        D_800B8F24.m[0][2] = 0x80;
        D_800B8F24.m[1][2] = 0x80;
        D_800B8F24.m[2][2] = 0x80;
        D_800B8F24.m[0][0] = 0;
        D_800B8F24.m[0][1] = 0;
        D_800B8F24.m[1][0] = 0;
        D_800B8F24.m[1][1] = 0;
        D_800B8F24.m[2][0] = 0;
        D_800B8F24.m[2][1] = 0;
        D_800B8F84.m[0][2] = 0;
        D_800B8F84.m[1][2] = 0x80;
        D_800B8F84.m[2][2] = 0;
        D_800B8F84.m[0][0] = 0;
        D_800B8F84.m[0][1] = 0;
        D_800B8F84.m[1][0] = 0;
        D_800B8F84.m[1][1] = 0;
        D_800B8F84.m[2][0] = 0;
        D_800B8F84.m[2][1] = 0;
        D_800B8FA4.m[0][2] = 0x80;
        D_800B8FA4.m[1][2] = 0;
        D_800B8FA4.m[2][2] = 0;
        D_800B8FA4.m[0][0] = 0;
        D_800B8FA4.m[0][1] = 0;
        D_800B8FA4.m[1][0] = 0;
        D_800B8FA4.m[1][1] = 0;
        D_800B8FA4.m[2][0] = 0;
        D_800B9024.m[0][2] = 0x80;
        D_800B8FA4.m[2][1] = 0;
        D_800B9024.m[1][2] = 0x40;
        D_800B9024.m[2][2] = 0;
        D_800B9024.m[0][0] = 0;
        D_800B9024.m[0][1] = 0;
        D_800B9024.m[1][0] = 0;
        D_800B9024.m[1][1] = 0;
        D_800B9024.m[2][0] = 0;
        D_800B9024.m[2][1] = 0;
    }
    return o;
}
