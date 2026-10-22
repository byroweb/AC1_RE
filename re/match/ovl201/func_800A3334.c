#include "ac1/ac1.h"
/* ovl201 0x800A3334 - title menu (NEW GAME at y 0xB0 / CONTINUE at y 0xC0):
 * item_a's image follows the alt flag, up/down move the cursor by 16, and
 * START/X highlights the chosen item and goes to func_800A3554. */
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
} TitleWork;

typedef struct {
    u32 used;           /* +0x00 */
    s16 delay;
    s16 rate;
    u8 pad08[0x10];
    u32 press;          /* +0x18 */
    u32 prev;           /* +0x1C */
} PadState;
extern PadState D_801A2550;
extern s16 D_801A2318;
extern u8 D_800BDC08[];
extern u8 D_800BDC14[];
extern s32 func_800309A8(s32 a, s32 b, s32 c);
extern void func_8009C928(void);
extern void func_800A3554(UiObj *o);

void func_800A3334(UiObj *o)
{
    TitleWork *w = (TitleWork *)&o->w;
    s32 alt;
    UiObj *sel;

    alt = func_800309A8(0, 0, 0) & 0x80;
    func_8009C928();
    if (D_801A2550.press & 2) {
        alt = 1;
    }
    if (alt == 0) {
        ((TitleWork *)&o->w)->item_a->res = D_800BDC14;
    } else {
        ((TitleWork *)&o->w)->item_a->res = D_800BDC08;
    }
    if ((D_801A2550.press & 0x1000) && !(D_801A2550.prev & 0x1000) && w->cursor->pos.vy > 0xB0) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        w->cursor->pos.vy -= 0x10;
        return;
    }
    if ((D_801A2550.press & 0x4000) && !(D_801A2550.prev & 0x4000) && w->cursor->pos.vy < 0xC0) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        w->cursor->pos.vy += 0x10;
        return;
    }
    if (D_801A2550.press & 0x840) {
        D_801A2550.used |= D_801A2550.press;
        if (w->cursor->pos.vy == 0xB0) {
            w->item_a->light = 0x10;
        } else if (w->cursor->pos.vy == 0xC0) {
            w->item_b->light = 0x10;
        }
        o->update = func_800A3554;
    }
}
