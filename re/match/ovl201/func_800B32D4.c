#include "ac1/ac1.h"

typedef struct RankWork {
    u8 pad7c[0x1C];
    u8 page;                    /* +0x98 */
    u8 row;                     /* +0x99 */
    u8 pad9a[2];
    s32 handle;                 /* +0x9C */
} RankWork;

extern s32 func_800517C8(void);
extern UiObj *func_8005CAE0(void *, s32, s32, s32);
extern void func_8005CB94(s32, s32, s32, s32);
extern UiObj *func_8005D754(s32, void *, s32, s32, s32);
extern void func_800B3414(void);
extern void func_8005BA2C();
extern void func_800B3E98(UiObj *);
extern u8 D_800BDEFC[];
extern u8 D_800BDF08[];
extern u8 D_8017F58C[];

void func_800B32D4(UiObj *o)
{
    RankWork *w;
    s32 i;

    i = o->w.raw[0x1D] + o->w.raw[0x1C];
    w = (RankWork *)&o->w;
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none && (s16)func_800517C8() == 0) {
        if (D_8017F58C[(o->w.raw[0x1D] + o->w.raw[0x1C]) * 16] != 0xF) {
            func_8005CAE0(D_800BDEFC, 0, 0, 8)->render_type = 4;
        } else {
            func_8005D754(0x400, D_800BDEFC, 0, 0, 8)->render_type = 4;
            func_800B3414();
        }
        func_8005CAE0(D_800BDF08 + D_8017F58C[i * 16] * 12, 8, 8, 10);
        func_8005CB94(w->handle, 0x50, 8, 10);
        o->update = func_800B3E98;
    }
}
