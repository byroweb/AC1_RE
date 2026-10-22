#include "ac1/ac1.h"

extern void func_8005D754(s32 a, void *p, s32 x, s32 y, s32 n);
extern void func_8005D80C(s32 a, void *s, s32 b, s32 c, s32 x, s32 y);
extern void func_80056EF4(u8 *col, u8 *row);
extern void func_8005616C(UiObj *o);
extern void func_8005BA2C();
extern u8 D_8004AFA0[];
extern u8 D_8004AFAC[];
extern u8 D_800B6F58[][12];

void func_80056034(UiObj *o)
{
    s16 x;
    s16 y;
    s16 i;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        if (o->w.raw[0x29] == 'S') {
            func_8005D80C(2, D_8004AFA0, 0x10, 8, 4, 4);
        } else {
            func_8005D80C(2, D_8004AFAC, 0x10, 8, 4, 4);
        }
        x = 8;
        y = 8;
        for (i = 0; i < 8; i++) {
            func_8005D754(2, D_800B6F58[i], x, y, 6);
            x += 0x44;
            if (x == 0x118) {
                x = 8;
                y += 0x40;
            }
        }
        func_80056EF4(&o->w.raw[0x2C], &o->w.raw[0x2D]);
        o->update = func_8005616C;
    }
}
