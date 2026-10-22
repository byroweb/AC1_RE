#include "ac1/ac1.h"
/* ovl201 0x8008FA94 - shop part list input (once no func_8005C2D0 window
 * is opening). The two arrows show rows above / below the window.
 * Confirm (0x40): buy - a dialog when the part costs more than the credits,
 * else func_80093EE8; sell - with negative credits an equipped / needed part
 * opens func_8008F978's dialog, else func_80093EE8. Cancel (0x20): close the
 * list, stats and description windows and go back to the category list
 * (buy, saving cursor/top per category) or the BUY/SELL menu (sell).
 * Pad 0x10 toggles the stats (func_80090C9C) and description
 * (func_80093B58) windows. */

typedef struct {
    u8 pad00;
    s8 mode;            /* +0x01 0 = buy list */
    s8 kind;            /* +0x02 selected category */
    u8 pad03;
    s8 rows;            /* +0x04 visible rows */
    u8 pad05[0x53];
    s16 n;              /* +0x58 rows in the current list */
    s16 top;            /* +0x5A */
    s16 total;          /* +0x5C */
    u16 sell_top;       /* +0x5E */
    u16 saved_top[10];  /* +0x60 per category */
    s16 cur;            /* +0x74 */
    u16 sell_cur;       /* +0x76 */
    u16 saved_cur[10];  /* +0x78 per category */
    u8 pad8c[0x30];
    UiObj *menu_cursor[2];      /* +0xBC */
    UiObj *cat_cursor[4];       /* +0xC4 */
    UiObj *cursor[3];           /* +0xD4 */
    UiObj *menu_label[2];       /* +0xE0 */
    UiObj *cat_label[4];        /* +0xE8 */
    UiObj *name_label[3];       /* +0xF8 */
    UiObj *price_label[3];      /* +0x104 */
    u8 pad110[0xC];
    s8 kinds[0xA0];     /* +0x11C */
    s8 ids[0xA0];       /* +0x1BC */
} ShopState;
extern ShopState D_8019C090;
extern s32 D_80039CA4;
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
extern void func_80095784(void);
extern void func_80090C9C(void);
extern void func_80093B58(void);
extern void func_80090B90(void);
extern void func_800902EC(void);
extern void func_8008DD50(void);
extern void func_800953AC(void);
extern void func_8008C3EC(void);
extern void func_80095130(void);
extern void func_8008C1F8(void);
extern void func_8005C2D0(s32 a, s32 x, s32 y, s32 h, s32 id);
extern void func_800506EC(s32 n);
extern void func_800AE30C(void);
extern void func_80093EE8(void);
extern void func_8008F978(void);
extern u8 func_8008F914(void);
extern u8 func_800892AC(void);
extern u8 func_8008935C(void);
extern u8 func_800895D4(void);
extern s32 func_8008FA00(u8 kind, u16 id);
extern void func_80095F4C(UiObj *o);
extern void func_800902A4(UiObj *o);
extern void func_80093B4C(UiObj *o);
extern void func_80093EA0(UiObj *o);
extern void func_80090C58(UiObj *o);
extern void func_80090400(UiObj *o);
extern void func_8008E260(UiObj *o);
extern void func_8009544C(UiObj *o);
extern void func_8008D560(UiObj *o);
extern void func_800951B0(UiObj *o);

void func_8008FA94(UiObj *o)
{
    UiObj *list;
    UiObj *stats;
    UiObj *desc;
    UiObj *model;
    UiObj *texture;
    UiObj *a;
    UiObj *b;
    UiObj *c;
    UiObj *d;
    UiObj *p;
    UiObj *q;
    UiObj *x;
    UiObj *y;
    u8 k;
    s16 id;

    list = obj_find_by_handle(func_80095784);
    stats = obj_find_by_handle(func_80090C9C);
    desc = obj_find_by_handle(func_80093B58);
    if (obj_find_by_handle(func_8005C2D0) == &g_ui_none) {
        if (D_8019C090.n < 4) {
            o->w.ring.item[3]->visible = 0;
            o->w.ring.item[4]->visible = 0;
        } else {
            if (D_8019C090.top - list->pos.vy / 24 > 0) {
                o->w.ring.item[3]->visible = 1;
            } else {
                o->w.ring.item[3]->visible = 0;
            }
            if (D_8019C090.top - list->pos.vy / 24 != D_8019C090.n - 3) {
                o->w.ring.item[4]->visible = 1;
            } else {
                o->w.ring.item[4]->visible = 0;
            }
        }
        if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) {
            o->w.ring.item[3]->kill = 1;
            o->w.ring.item[4]->kill = 1;
            list->light = 16;
            list->update = ui_obj_nop2;
            D_8019C090.cursor[0]->light = 16;
            D_8019C090.cursor[1]->light = 16;
            D_8019C090.cursor[2]->light = 16;
            D_8019C090.name_label[0]->light = 16;
            D_8019C090.name_label[1]->light = 16;
            D_8019C090.name_label[2]->light = 16;
            D_8019C090.price_label[0]->light = 16;
            D_8019C090.price_label[1]->light = 16;
            D_8019C090.price_label[2]->light = 16;
            o->update = ui_obj_nop2;
            stats->update = ui_obj_nop2;
            desc->update = ui_obj_nop2;
            if (D_8019C090.mode == 0) {
                if (func_8008F914()) {
                    D_801A2314 = func_8008C1F8;
                    D_801A230B = 10;
                    D_801A230C = 14;
                    D_801A230D = 13;
                    D_801A230E = 6;
                    D_801A2309 = 1;
                    D_801A230A = 0;
                    D_801A2310 = D_800B8C98;
                    func_800AE30C();
                    return;
                }
            } else if (D_80039CA4 < 0) {
                k = D_8019C090.kinds[D_8019C090.top];
                id = D_8019C090.ids[D_8019C090.top];
                if (k != 6 && (u8)(k - 8) >= 2 &&
                    (func_800892AC() || func_8008935C() || func_800895D4() || func_8008FA00(k, id))) {
                    func_8008F978();
                    return;
                }
            }
            func_80093EE8();
        } else if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
            model = obj_find_by_handle(func_80090B90);
            texture = obj_find_by_handle(func_800902EC);
            p = obj_find_by_handle(func_80090C9C);
            q = obj_find_by_handle(func_80093B58);
            o->w.ring.item[3]->kill = 1;
            o->w.ring.item[4]->kill = 1;
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            list->update = func_80095F4C;
            func_800506EC(10);
            obj_find_by_id(9)->kill = 1;
            func_8005C2D0(140, 24, 176, D_8019C090.rows * 24, 9);
            o->update = func_800902A4;
            if (p != &g_ui_none) {
                func_800506EC(12);
                obj_find_by_id(11)->kill = 1;
                p->update = func_80093B4C;
            }
            if (q != &g_ui_none) {
                func_800506EC(18);
                obj_find_by_id(17)->kill = 1;
                func_8005C2D0(8, 172, 304, 24, 17);
                q->update = func_80093EA0;
            }
            model->update = func_80090C58;
            func_800506EC(8);
            obj_find_by_id(7)->kill = 1;
            func_8005C2D0(168, 104, 144, 112, 7);
            texture->update = func_80090400;
            if (D_8019C090.mode == 0) {
                a = obj_find_by_handle(func_8008DD50);
                b = obj_find_by_handle(func_800953AC);
                D_8019C090.cat_cursor[0]->light = 21;
                D_8019C090.cat_cursor[1]->light = 21;
                D_8019C090.cat_cursor[2]->light = 21;
                D_8019C090.cat_cursor[3]->light = 21;
                D_8019C090.cat_label[0]->light = 20;
                D_8019C090.cat_label[1]->light = 20;
                D_8019C090.cat_label[2]->light = 20;
                D_8019C090.cat_label[3]->light = 20;
                a->update = func_8008E260;
                b->light = 21;
                b->update = func_8009544C;
                D_8019C090.saved_top[D_8019C090.kind] = D_8019C090.top;
                D_8019C090.saved_cur[D_8019C090.kind] = D_8019C090.cur;
            } else {
                c = obj_find_by_handle(func_8008C3EC);
                d = obj_find_by_handle(func_80095130);
                D_8019C090.menu_cursor[0]->light = 21;
                D_8019C090.menu_cursor[1]->light = 21;
                D_8019C090.menu_label[0]->light = 20;
                D_8019C090.menu_label[1]->light = 20;
                c->update = func_8008D560;
                d->light = 21;
                d->update = func_800951B0;
                D_8019C090.sell_top = D_8019C090.top;
                D_8019C090.sell_cur = D_8019C090.cur;
            }
        } else if ((D_801A2568 & 0x10) && !(D_801A256C & 0x10)) {
            x = obj_find_by_handle(func_80090C9C);
            y = obj_find_by_handle(func_80093B58);
            o->w.ring.item[3]->visible = 0;
            o->w.ring.item[4]->visible = 0;
            if (x != &g_ui_none) {
                func_800506EC(12);
                obj_find_by_id(11)->kill = 1;
                x->update = func_80093B4C;
            } else {
                func_80090C9C();
            }
            if (y != &g_ui_none) {
                func_800506EC(18);
                obj_find_by_id(17)->kill = 1;
                y->update = func_80093EA0;
            } else {
                func_80093B58();
            }
        }
    }
}
