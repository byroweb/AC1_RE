#include "ac1/ac1.h"
/* ovl201 0x80051F24 - once no func_8005BA2C object is left: show the message, play the cue, go to func_80051FB0 */
typedef struct MsgWork {
    u8 pad7c[8];
    u8 *text;                   /* +0x84 */
    s16 y;                      /* +0x88 */
    u8 pad8a[2];
    UiObj *label;               /* +0x8C */
} MsgWork;
extern s16 D_801A2318;
void func_8005BA2C(UiObj *o);
void func_80051FB0(UiObj *o);
UiObj *func_8005CB94(u8 *s, s32 x, s32 y, s32 c);
s16 SsUtKeyOn(s16 vab, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr);

void func_80051F24(UiObj *o)
{
    MsgWork *w;

    w = (MsgWork *)&o->w;
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        w->label = func_8005CB94(w->text, 8, 8, w->y);
        SsUtKeyOn(D_801A2318, 0, 7, 60, 0, 127, 127);
        o->update = func_80051FB0;
    }
}
