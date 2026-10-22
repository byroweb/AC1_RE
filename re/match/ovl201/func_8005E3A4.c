#include "ac1/ac1.h"

typedef struct {
    u8 pad7c[0x10];
    u16 id;                     /* +0x8C */
    u8 pad8e[2];
    UiObj *child;               /* +0x90 */
} Work21;

extern void func_8005C2D0(s32, s32, s32, s32, s32);
typedef struct { s32 used; s16 d; s16 r; u8 pad[0x10]; s32 press; s32 held; } PadSt;
extern PadSt D_801A2550;
extern void func_8005E4D4(UiObj *);

void func_8005E3A4(UiObj *o) {
    Work21 *w = (Work21 *)&o->w;

    if (D_801A2550.press & 0x600060) {
        if (D_801A2550.press & 0x40) {
            D_801A2550.used |= 0x40;
        }
        if (D_801A2550.press & 0x20) {
            D_801A2550.used |= 0x20;
        }
        if (D_801A2550.press & 0x400000) {
            D_801A2550.used |= 0x400000;
        }
        if (D_801A2550.press & 0x200000) {
            D_801A2550.used |= 0x200000;
        }
        w->child->kill = 1;
        obj_find_by_id((s16)(w->id - 1))->kill = 1;
        func_8005C2D0(0x14, 0x68, 0x118, 0x20, (s16)(w->id - 1));
        o->update = func_8005E4D4;
    }
}
