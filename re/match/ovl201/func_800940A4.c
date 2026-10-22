#include "ac1/ac1.h"
/* ovl201 0x800940A4 - shop buy/sell yes/no dialog input. Confirm (0x40)
 * on YES (dialog cursor pos.vx == 4): buying adds one to the owned count
 * (0x80031A94) and takes one from the shop stock (0x80031B34), pays the
 * price (credits floor -999999), closes the windows, rebuilds the buy list
 * and returns to the category list; selling does the reverse (credits cap
 * 99999999), rebuilds the sell list and either goes back to the BUY/SELL
 * menu (list now empty) or redraws the list. NO / cancel (0x20) closes the
 * dialog and returns to the part list. */

typedef struct {
    u8 pad00;
    s8 mode;            /* +0x01 0 = buy list */
    s8 kind;            /* +0x02 selected category */
    u8 pad03;
    s8 rows;            /* +0x04 visible rows */
    u8 pad05[0x55];
    s16 top;            /* +0x5A */
    s16 total;          /* +0x5C */
    u16 sell_top;       /* +0x5E */
    u16 saved_top[10];  /* +0x60 per category */
    s16 cur;            /* +0x74 */
    u16 sell_cur;       /* +0x76 */
    u16 saved_cur[10];  /* +0x78 per category */
    s8 count[10];       /* +0x8C */
    u16 start[10];      /* +0x96 */
    u8 padAA[0x12];
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
extern u8 D_80031A94[];
extern u8 D_80031B34[];
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s16 D_801A2318;
extern u8 D_800B91AC[];
extern u8 D_800B91B8[];
extern void func_80095F58(void);
extern void func_80095784(void);
extern void func_8008E70C(void);
extern void func_80090C9C(void);
extern void func_80093B58(void);
extern void func_80090B90(void);
extern void func_800902EC(void);
extern void func_8008DD50(void);
extern void func_800953AC(void);
extern void func_8008C3EC(void);
extern void func_80095130(void);
extern s16 func_8008CFD4(s8 kind, s8 idx);
extern s32 func_8008F08C(u8 kind, u8 idx);
extern void func_8009406C(void);
extern void func_8008D060(void);
extern void func_8008D2E0(void);
extern void func_80090668(void);
extern void func_800908F8(void);
extern void func_8005C2D0(s32 a, s32 x, s32 y, s32 h, s32 id);
extern void func_800506EC(s32 n);
extern UiObj *func_8005CAE0(void *res, s32 x, s32 y, s32 z);
extern void func_800960B4(UiObj *o);
extern void func_80094DC0(UiObj *o);
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
extern void func_8008FA94(UiObj *o);
extern void func_80095824(UiObj *o);
extern void func_80093AE8(UiObj *o);
extern void func_80093E3C(UiObj *o);

void func_800940A4(UiObj *o)
{
    UiObj *dlg;
    UiObj *list;
    UiObj *rows;
    UiObj *stats;
    UiObj *desc;
    UiObj *a;
    UiObj *b;
    UiObj *c;
    UiObj *d;
    UiObj *e;
    UiObj *f;
    UiObj *g;
    UiObj *h;
    UiObj *p;
    UiObj *q;
    UiObj *r;
    UiObj *s;
    UiObj *t;
    s16 i;
    UiObj *x;

    dlg = obj_find_by_handle(func_80095F58);
    list = obj_find_by_handle(func_80095784);
    rows = obj_find_by_handle(func_8008E70C);
    stats = obj_find_by_handle(func_80090C9C);
    desc = obj_find_by_handle(func_80093B58);
    if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (D_8019C090.mode == 0) {
            if (dlg->pos.vx != 4) {
            dlg->update = func_800960B4;
            func_800506EC(14);
            obj_find_by_id(13)->kill = 1;
            func_8005C2D0(96, 100, 144, 48, 13);
            o->update = func_80094DC0;
            D_8019C090.cursor[0]->light = 21;
            D_8019C090.cursor[1]->light = 21;
            D_8019C090.cursor[2]->light = 21;
            D_8019C090.name_label[0]->light = 20;
            D_8019C090.name_label[1]->light = 20;
            D_8019C090.name_label[2]->light = 20;
            D_8019C090.price_label[0]->light = 20;
            D_8019C090.price_label[1]->light = 20;
            D_8019C090.price_label[2]->light = 20;
            rows->update = func_8008FA94;
            list->light = 21;
            list->update = func_80095824;
            if (stats != &g_ui_none) {
                stats->update = func_80093AE8;
            }
            if (desc != &g_ui_none) {
                desc->update = func_80093E3C;
            }
                return;
            }
            i = D_8019C090.top + D_8019C090.start[D_8019C090.kind];
            if (D_80031A94[func_8008CFD4(D_8019C090.kind, D_8019C090.ids[i])] != 127) {
                D_80031A94[func_8008CFD4(D_8019C090.kind, D_8019C090.ids[i])]++;
            }
            if (D_80031B34[func_8008CFD4(D_8019C090.kind, D_8019C090.ids[i])] != 127) {
                D_80031B34[func_8008CFD4(D_8019C090.kind, D_8019C090.ids[i])]--;
            }
            if (D_80039CA4 - func_8008F08C(D_8019C090.kind, D_8019C090.ids[i]) < -999999) {
                D_80039CA4 = -999999;
            } else {
                D_80039CA4 -= func_8008F08C(D_8019C090.kind, D_8019C090.ids[i]);
            }
            func_8009406C();
            dlg->update = func_800960B4;
            func_800506EC(14);
            obj_find_by_id(13)->kill = 1;
            func_8005C2D0(96, 100, 144, 48, 13);
            o->update = func_80094DC0;
            list->update = func_80095F4C;
            func_800506EC(10);
            obj_find_by_id(9)->kill = 1;
            func_8005C2D0(140, 24, 176, D_8019C090.rows * 24, 9);
            rows->update = func_800902A4;
            if (stats != &g_ui_none) {
                func_800506EC(12);
                obj_find_by_id(11)->kill = 1;
                stats->update = func_80093B4C;
            }
            if (desc != &g_ui_none) {
                func_800506EC(18);
                obj_find_by_id(17)->kill = 1;
                desc->update = func_80093EA0;
            }
            func_8008D060();
            if (D_8019C090.count[D_8019C090.kind] >= 4) {
                D_8019C090.rows = 3;
            } else {
                D_8019C090.rows = D_8019C090.count[D_8019C090.kind];
            }
            a = obj_find_by_handle(func_800953AC);
            b = obj_find_by_handle(func_8008DD50);
            c = obj_find_by_handle(func_80090B90);
            d = obj_find_by_handle(func_800902EC);
            e = obj_find_by_handle(func_80090C9C);
            f = obj_find_by_handle(func_80093B58);
            if (e != &g_ui_none) {
                func_800506EC(12);
                obj_find_by_id(11)->kill = 1;
                e->update = func_80093B4C;
            }
            if (f != &g_ui_none) {
                func_800506EC(18);
                obj_find_by_id(17)->kill = 1;
                func_8005C2D0(8, 172, 304, 24, 17);
                f->update = func_80093EA0;
            }
            c->update = func_80090C58;
            func_800506EC(8);
            obj_find_by_id(7)->kill = 1;
            func_8005C2D0(168, 104, 144, 112, 7);
            d->update = func_80090400;
            D_8019C090.cat_cursor[0]->light = 21;
            D_8019C090.cat_cursor[1]->light = 21;
            D_8019C090.cat_cursor[2]->light = 21;
            D_8019C090.cat_cursor[3]->light = 21;
            D_8019C090.cat_label[0]->light = 20;
            D_8019C090.cat_label[1]->light = 20;
            D_8019C090.cat_label[2]->light = 20;
            D_8019C090.cat_label[3]->light = 20;
            b->update = func_8008E260;
            a->light = 21;
            a->update = func_8009544C;
            D_8019C090.saved_top[D_8019C090.kind] = D_8019C090.top;
            D_8019C090.saved_cur[D_8019C090.kind] = D_8019C090.cur;
            return;
        }
        if (dlg->pos.vx != 4) {
        dlg->update = func_800960B4;
        func_800506EC(14);
        obj_find_by_id(13)->kill = 1;
        func_8005C2D0(96, 100, 144, 48, 13);
        o->update = func_80094DC0;
        D_8019C090.cursor[0]->light = 21;
        D_8019C090.cursor[1]->light = 21;
        D_8019C090.cursor[2]->light = 21;
        D_8019C090.name_label[0]->light = 20;
        D_8019C090.name_label[1]->light = 20;
        D_8019C090.name_label[2]->light = 20;
        D_8019C090.price_label[0]->light = 20;
        D_8019C090.price_label[1]->light = 20;
        D_8019C090.price_label[2]->light = 20;
        rows->update = func_8008FA94;
        list->light = 21;
        list->update = func_80095824;
        if (stats != &g_ui_none) {
            stats->update = func_80093AE8;
        }
        if (desc != &g_ui_none) {
            desc->update = func_80093E3C;
        }
            return;
        }
        if (D_80031A94[func_8008CFD4(D_8019C090.kinds[D_8019C090.top], D_8019C090.ids[D_8019C090.top])] != 127) {
            D_80031A94[func_8008CFD4(D_8019C090.kinds[D_8019C090.top], D_8019C090.ids[D_8019C090.top])]--;
        }
        if (D_80031B34[func_8008CFD4(D_8019C090.kinds[D_8019C090.top], D_8019C090.ids[D_8019C090.top])] != 127) {
            D_80031B34[func_8008CFD4(D_8019C090.kinds[D_8019C090.top], D_8019C090.ids[D_8019C090.top])]++;
        }
        if (D_80039CA4 + func_8008F08C(D_8019C090.kinds[D_8019C090.top], D_8019C090.ids[D_8019C090.top]) > 99999998) {
            D_80039CA4 = 99999999;
        } else {
            D_80039CA4 += func_8008F08C(D_8019C090.kinds[D_8019C090.top], D_8019C090.ids[D_8019C090.top]);
        }
        func_8009406C();
        dlg->update = func_800960B4;
        func_800506EC(14);
        obj_find_by_id(13)->kill = 1;
        func_8005C2D0(96, 100, 144, 48, 13);
        o->update = func_80094DC0;
        list->update = func_80095F4C;
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(140, 24, 176, D_8019C090.rows * 24, 9);
        rows->update = func_800902A4;
        g = obj_find_by_handle(func_80090C9C);
        h = obj_find_by_handle(func_80093B58);
        if (g != &g_ui_none) {
            func_800506EC(12);
            obj_find_by_id(11)->kill = 1;
            g->update = func_80093B4C;
        }
        if (h != &g_ui_none) {
            func_800506EC(18);
            obj_find_by_id(17)->kill = 1;
            func_8005C2D0(8, 172, 304, 24, 17);
            h->update = func_80093EA0;
        }
        func_8008D2E0();
        if (D_8019C090.total >= 4) {
            D_8019C090.rows = 3;
        } else {
            D_8019C090.rows = D_8019C090.total;
        }
        if (D_8019C090.rows <= 0) {
            p = obj_find_by_handle(func_80090B90);
            q = obj_find_by_handle(func_800902EC);
            r = obj_find_by_handle(func_8008C3EC);
            s = obj_find_by_handle(func_80095130);
            p->update = func_80090C58;
            func_800506EC(8);
            obj_find_by_id(7)->kill = 1;
            func_8005C2D0(168, 104, 144, 112, 7);
            q->update = func_80090400;
            D_8019C090.menu_cursor[0]->light = 21;
            D_8019C090.menu_cursor[1]->light = 21;
            D_8019C090.menu_label[0]->light = 20;
            D_8019C090.menu_label[1]->light = 20;
            r->update = func_8008D560;
            s->light = 21;
            s->update = func_800951B0;
            D_8019C090.sell_top = D_8019C090.top;
            D_8019C090.sell_cur = D_8019C090.cur;
            return;
        }
        if (D_8019C090.cur > D_8019C090.total - D_8019C090.rows) {
            D_8019C090.cur = D_8019C090.total - D_8019C090.rows;
        }
        if (D_8019C090.top > D_8019C090.total - 1) {
            D_8019C090.top = D_8019C090.total - 1;
        }
        func_80090668();
        func_800908F8();
        func_8008E70C();
        return;
    }
    if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        t = obj_find_by_handle(func_8008E70C);
        t->w.ring.item[3] = func_8005CAE0(D_800B91AC, 114, 26, 18);
        obj_find_by_handle(func_8008E70C)->w.ring.item[3]->light = 11;
        obj_find_by_handle(func_8008E70C)->w.ring.item[3]->visible = 0;
        x = obj_find_by_handle(func_8008E70C);
        x->w.ring.item[4] = func_8005CAE0(D_800B91B8, 114, D_8019C090.rows * 24 + 8, 18);
        obj_find_by_handle(func_8008E70C)->w.ring.item[4]->light = 11;
        obj_find_by_handle(func_8008E70C)->w.ring.item[4]->visible = 0;
        dlg->update = func_800960B4;
        func_800506EC(14);
        obj_find_by_id(13)->kill = 1;
        func_8005C2D0(96, 100, 144, 48, 13);
        o->update = func_80094DC0;
        D_8019C090.cursor[0]->light = 21;
        D_8019C090.cursor[1]->light = 21;
        D_8019C090.cursor[2]->light = 21;
        D_8019C090.name_label[0]->light = 20;
        D_8019C090.name_label[1]->light = 20;
        D_8019C090.name_label[2]->light = 20;
        D_8019C090.price_label[0]->light = 20;
        D_8019C090.price_label[1]->light = 20;
        D_8019C090.price_label[2]->light = 20;
        rows->update = func_8008FA94;
        list->light = 21;
        list->update = func_80095824;
        if (stats != &g_ui_none) {
            stats->update = func_80093AE8;
        }
        if (desc != &g_ui_none) {
            desc->update = func_80093E3C;
        }
    }
}
