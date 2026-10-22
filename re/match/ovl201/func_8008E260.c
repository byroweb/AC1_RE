#include "ac1/ac1.h"
/* ovl201 0x8008E260 - shop category list input. Confirm (0x40) dims the
 * list and opens the part list of the selected category (restoring that
 * category's cursor and top row), or a dialog when it is empty; cancel
 * (0x20) goes back to the BUY/SELL menu. Otherwise the two arrows show
 * whether rows lie above / below the visible window. */

typedef struct {
    u8 pad00[2];
    s8 kind;            /* +0x02 selected category */
    u8 pad03;
    s8 rows;            /* +0x04 visible rows (<= 3) */
    u8 pad05[0x55];
    s16 top;            /* +0x5A */
    u8 pad5c[4];
    u16 saved_top[10];  /* +0x60 per category */
    s16 cur;            /* +0x74 */
    u8 pad76[2];
    u16 saved_cur[10];  /* +0x78 per category */
    s8 count[10];       /* +0x8C */
    u8 pad96[0x26];
    UiObj *menu_cursor[2];      /* +0xBC */
    UiObj *cursor[4];           /* +0xC4 */
    u8 padD4[0xC];
    UiObj *menu_label[2];       /* +0xE0 */
    UiObj *label[4];            /* +0xE8 */
} ShopState;
extern ShopState D_8019C090;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s16 D_801A2318;
extern s8 D_801A2309;
extern s8 D_801A230A;
extern s8 D_801A230B;
extern s8 D_801A230C;
extern s8 D_801A230D;
extern s8 D_801A230E;
extern void *D_801A2310;
extern void *D_801A2314;
extern u8 D_800B8C98[];
extern void func_800953AC(void);
extern void func_80095130(void);
extern void func_8008C3EC(void);
extern void func_8008C120(void);
extern void func_8008E70C(void);
extern void func_800902EC(void);
extern void func_80090B90(void);
extern void func_800AE30C(void);
extern void func_800506EC(s32 n);
extern void func_8005C2D0(s32 a, s32 x, s32 y, s32 w, s32 id);
extern void func_80095778(UiObj *o);
extern void func_8008E6C4(UiObj *o);
extern void func_8008D560(UiObj *o);
extern void func_800951B0(UiObj *o);

void func_8008E260(UiObj *o)
{
    UiObj *list;
    UiObj *menu;
    UiObj *ctl;

    list = obj_find_by_handle(func_800953AC);
    if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) {
        ((UiObj *)o->w.list.src)->visible = 0;
        o->w.ring.item[0]->visible = 0;
        if (D_8019C090.count[D_8019C090.kind] >= 4) {
            D_8019C090.rows = 3;
        } else {
            D_8019C090.rows = D_8019C090.count[D_8019C090.kind];
        }
        list->light = 16;
        list->update = ui_obj_nop2;
        D_8019C090.cursor[0]->light = 16;
        D_8019C090.cursor[1]->light = 16;
        D_8019C090.cursor[2]->light = 16;
        D_8019C090.cursor[3]->light = 16;
        D_8019C090.label[0]->light = 16;
        D_8019C090.label[1]->light = 16;
        D_8019C090.label[2]->light = 16;
        D_8019C090.label[3]->light = 16;
        o->update = ui_obj_nop2;
        if (D_8019C090.rows <= 0) {
            D_801A2309 = 1;
            D_801A230A = 1;
            D_801A2314 = func_8008C120;
            D_801A230B = 10;
            D_801A230C = 14;
            D_801A230D = 13;
            D_801A230E = 6;
            D_801A2310 = D_800B8C98;
            func_800AE30C();
        } else {
            D_8019C090.cur = D_8019C090.saved_cur[D_8019C090.kind];
            if (D_8019C090.cur > D_8019C090.count[D_8019C090.kind] - D_8019C090.rows) {
                D_8019C090.cur = D_8019C090.count[D_8019C090.kind] - D_8019C090.rows;
            }
            D_8019C090.top = D_8019C090.saved_top[D_8019C090.kind];
            if (D_8019C090.top > D_8019C090.count[D_8019C090.kind] - 1) {
                D_8019C090.top = D_8019C090.count[D_8019C090.kind] - 1;
            }
            func_8008E70C();
            func_800902EC();
            func_80090B90();
        }
    } else if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
        menu = obj_find_by_handle(func_80095130);
        ctl = obj_find_by_handle(func_8008C3EC);
        ((UiObj *)o->w.list.src)->kill = 1;
        o->w.ring.item[0]->kill = 1;
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        list->update = func_80095778;
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_8005C2D0(44, 92, 112, 96, 5);
        o->update = func_8008E6C4;
        D_8019C090.menu_cursor[0]->light = 21;
        D_8019C090.menu_cursor[1]->light = 21;
        D_8019C090.menu_label[0]->light = 20;
        D_8019C090.menu_label[1]->light = 20;
        ctl->update = func_8008D560;
        menu->light = 21;
        menu->update = func_800951B0;
    } else {
        if (D_8019C090.kind - list->pos.vy / 24 > 0) {
            ((UiObj *)o->w.list.src)->visible = 1;
        } else {
            ((UiObj *)o->w.list.src)->visible = 0;
        }
        if (list->pos.vy / 24 != D_8019C090.kind - 6) {
            o->w.ring.item[0]->visible = 1;
        } else {
            o->w.ring.item[0]->visible = 0;
        }
    }
}
