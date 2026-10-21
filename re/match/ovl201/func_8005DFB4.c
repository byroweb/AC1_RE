#include "ac1/ac1.h"

/* UiObj work area of this class (offsets are UiObj offsets). */
typedef struct {
    u8 pad7c[0x0C];
    s32 res;                    /* +0x88 */
    s16 arg;                    /* +0x8C */
    u8 pad8e[2];
    s32 handle;                 /* +0x90 */
} Work21;

extern s32 func_8005CB94(s32, s32, s32, s16);
extern s16 D_801A2318;
extern void func_8005BA2C(UiObj *);
extern void func_8005E040(UiObj *);

void func_8005DFB4(UiObj *o) {
    Work21 *w = (Work21 *)&o->w;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        w->handle = func_8005CB94(w->res, 8, 8, w->arg);
        SsUtKeyOn(D_801A2318, 0, 7, 0x3C, 0, 0x7F, 0x7F);
        o->update = func_8005E040;
    }
}
