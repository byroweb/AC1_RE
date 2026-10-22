#include "ac1/ac1.h"
/* ovl201 0x80051FB0 - message box: wait for a confirm/cancel press, latch it, close the frame */
typedef struct MsgWork {
    UiObj *owner;               /* +0x7C */
    void *next;                 /* +0x80 owner's update on close */
    u8 *text;                   /* +0x84 */
    s16 id;                     /* +0x88 frame object id + 1 */
    u8 pad8a[2];
    UiObj *label;               /* +0x8C */
    s16 x, y, w, h;             /* +0x90 */
} MsgWork;
typedef struct {
    u32 used;
    s16 delay;
    s16 rate;
    u8 pad8[0x10];
    u32 press;
    u32 held;
} PadState;
extern PadState D_801A2550;
void func_800520E0(UiObj *o);
void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);

void func_80051FB0(UiObj *o)
{
    MsgWork *m = (MsgWork *)&o->w;
    s32 f;

    if (D_801A2550.press & 0x600060) {
        if (D_801A2550.press & 0x20) {
            D_801A2550.used |= 0x20;
        }
        f = D_801A2550.press;
        if (f & 0x40) {
            D_801A2550.used |= 0x40;
        }
        if (f & 0x200000) {
            D_801A2550.used |= 0x200000;
        }
        if (D_801A2550.press & 0x400000) {
            D_801A2550.used |= 0x400000;
        }
        m->label->kill = 1;
        obj_find_by_id((s16)(m->id - 1))->kill = 1;
        func_8005C2D0(m->x, m->y, m->w, m->h, (s16)(m->id - 1));
        o->update = func_800520E0;
    }
}
