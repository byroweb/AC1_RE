#include "ac1/ac1.h"

/* pad block at 0x801A2550 */
typedef struct {
    u32 used;           /* 0x801A2550 buttons consumed this frame */
    s16 delay1;         /* 0x801A2554 first repeat delay */
    s16 delay2;         /* 0x801A2556 */
    u8 pad08[0x10];
    u32 press;          /* 0x801A2568 */
    u32 held;           /* 0x801A256C */
} PadState;

typedef struct {
    u8 pad7c[8];
    u8 kind;            /* +0x84 'M' master volume, else music volume */
    u8 vol;             /* +0x85 0..63 */
} VolWork;

typedef struct {
    s16 seq[14];        /* 0x801BC454 SEQ access numbers */
    u8 cur;             /* 0x801BC470 */
} SeqState;

extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern u8 D_80048661;           /* master volume / 2 */
extern u8 D_80048662;           /* music volume / 2 */
extern SeqState D_801BC454;
extern void func_800AE838(void);
extern void func_800B02D0(UiObj *o);

#define W(o) ((VolWork *)&(o)->w)

void func_800B0048(UiObj *o)
{
    VolWork *w = W(o);

    if (D_801A2550.press & 0x2000) {
        D_801A2550.used |= 0x2000;
        if (D_801A2574 != 0) {
            D_801A2550.delay1 = 0;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.delay2 = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay1 = 0;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.delay2 = 12;
            }
        }
        if (w->vol != 63) {
            w->vol++;
        }
        if (w->kind == 'M') {
            SsSetMVol(w->vol * 2, w->vol * 2);
        } else {
            SsSeqSetVol(D_801BC454.seq[D_801BC454.cur], w->vol * 2, w->vol * 2);
        }
    } else if (D_801A2550.press & 0x8000) {
        D_801A2550.used |= 0x8000;
        if (D_801A2574 != 0) {
            D_801A2550.delay1 = 0;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.delay2 = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay1 = 0;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.delay2 = 12;
            }
        }
        if (w->vol != 0) {
            w->vol--;
        }
        if (w->kind == 'M') {
            SsSetMVol(w->vol * 2, w->vol * 2);
        } else {
            SsSeqSetVol(D_801BC454.seq[D_801BC454.cur], w->vol * 2, w->vol * 2);
        }
    } else if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        o->update = func_800B02D0;
    } else if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        if (W(o)->kind == 'M') {
            D_80048661 = W(o)->vol;
        } else {
            D_80048662 = W(o)->vol;
        }
        func_800AE838();
        o->update = func_800B02D0;
    }
}
