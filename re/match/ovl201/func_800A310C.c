#include "ac1/ac1.h"
/* ovl201 0x800A310C - title screen "press start": blink the prompt (item3
 * visible 20 frames off / 40 on), after 500 frames go back to the attract
 * credits (func_800A2DBC); on START (or when +0x9F is set) build the
 * NEW GAME / CONTINUE menu and go to func_800A3334. */
typedef struct {
    UiObj *bg;          /* +0x7C */
    UiObj *logo;        /* +0x80 */
    s16 idle;           /* +0x84 */
    s16 blink;          /* +0x86 */
    u8 pad88[4];
    UiObj *prompt;      /* +0x8C */
    UiObj *cursor;      /* +0x90 */
    UiObj *item_a;      /* +0x94 */
    UiObj *item_b;      /* +0x98 */
    u8 pad9c[3];
    u8 skip;            /* +0x9F */
} TitleWork;

extern u32 D_801A2550;
extern u32 D_801A2568;
extern s8 D_80039CA8;
extern s16 D_801A2318;
extern u8 D_800BDBFC[];
extern u8 D_800BDC08[];
extern u8 D_800BDC38[];
extern void func_8009C928(void);
extern void func_80051450(s32 a, s32 b);
extern UiObj *func_8005CAE0(void *def, s32 x, s32 y, s32 z);
extern void func_800A2DBC(UiObj *o);
extern void func_800A3334(UiObj *o);

void func_800A310C(UiObj *o)
{
    TitleWork *w = (TitleWork *)&o->w;
    u32 *pad;

    func_8009C928();
    pad = &D_801A2568;
    if ((*pad & 0x800) || ((TitleWork *)&o->w)->skip != 0) {
        D_801A2550 |= 0x800;
        if (((TitleWork *)&o->w)->skip == 0) {
            SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        }
        if ((*pad & 0xC) == 0xC) {
            D_80039CA8 = 0;
        } else {
            D_80039CA8 = 1;
        }
        w->prompt->kill = 1;
        w->item_a = func_8005CAE0(D_800BDC08, 0x6C, 0xC0, 2);
        w->item_b = func_8005CAE0(D_800BDBFC, 0x6C, 0xB0, 2);
        w->cursor = func_8005CAE0(D_800BDC38, 0x60, 0xB0, 2);
        o->update = func_800A3334;
        return;
    }
    if (((TitleWork *)&o->w)->prompt->visible != 0) {
        if (++((TitleWork *)&o->w)->blink >= 40) {
            ((TitleWork *)&o->w)->blink = 0;
            ((TitleWork *)&o->w)->prompt->visible = 0;
            return;
        }
    } else {
        if (++((TitleWork *)&o->w)->blink >= 20) {
            ((TitleWork *)&o->w)->blink = 0;
            ((TitleWork *)&o->w)->prompt->visible = 1;
            return;
        }
    }
    if (++w->idle >= 500) {
        w->bg->kill = 1;
        w->logo->kill = 1;
        w->prompt->kill = 1;
        func_80051450(4, 2);
        w->idle = 0;
        o->update = func_800A2DBC;
    }
}
