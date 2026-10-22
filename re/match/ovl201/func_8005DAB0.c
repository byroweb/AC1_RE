#include "ac1/ac1.h"

typedef struct {
    UiObj *cursor;              /* +0x00 (o+0x7C) */
    UiObj *yes;                 /* +0x04 */
    UiObj *no;                  /* +0x08 */
    s16 sel;                    /* +0x0C (o+0x88) 1 = second choice */
    u8 unk0e[0x0A];
    s16 id;                     /* +0x18 (o+0x94) */
    s8 result;                  /* +0x1A (o+0x96) */
} UiYesNo;

extern s16 D_801A2318;
extern u32 D_801A2550;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern short SsUtKeyOn(short vabId, short prog, short tone, short note, short fine, short voll, short volr);
extern void func_800506EC(s16);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_8005DCDC(UiObj *);

/* Yes/no box: left/right move the cursor, the confirm buttons close it. */
void func_8005DAB0(UiObj *o) {
    UiYesNo *w;

    w = (UiYesNo *)&o->w;
    if ((D_801A2568 & 0x2000) && !(D_801A256C & 0x2000) && *(s16 *)((u8 *)o + 0x88) == 0) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        *(s16 *)((u8 *)o + 0x88) = 1;
    } else if ((D_801A2568 & 0x8000) && !(D_801A256C & 0x8000) && w->sel == 1) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        w->sel = 0;
    }
    *(s32 *)((u8 *)w->cursor + 0x48) = w->sel * 69 + 4;
    if (D_801A2568 & 0x60) {
        D_801A2550 |= D_801A2568;
        if (!(D_801A2568 & 0x20) && w->sel == 0) {
            w->result = 0;
            SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        } else {
            w->result = 1;
            SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        }
        func_800506EC(w->id);
        obj_find_by_id(w->id - 1)->kill = 1;
        w->cursor->kill = 1;
        w->yes->kill = 1;
        w->no->kill = 1;
        func_8005C2D0(88, 96, 144, 48, (s16)(w->id - 1));
        o->update = func_8005DCDC;
    }
}
