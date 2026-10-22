#include "ac1/ac1.h"
/* ovl201 0x8008D560 - BUY/SELL menu input. Pad 0x40 (confirm) dims the menu
 * and builds the buy list (mode 0) or the sell list; an empty sell list
 * opens a message dialog (func_800AE30C), otherwise the cursor and top row
 * are restored and clamped and the list objects are made. Pad 0x20 (cancel)
 * closes the menu windows and goes back via func_8008DCB0. */

typedef struct {
    u8 pad00;
    s8 mode;            /* +0x01 0 = buy, else sell */
    u8 pad02[2];
    s8 rows;            /* +0x04 visible rows (<= 3) */
    u8 pad05[0x55];
    s16 top;            /* +0x5A */
    s16 total;          /* +0x5C */
    u16 saved_top;      /* +0x5E */
    u8 pad60[0x14];
    s16 cur;            /* +0x74 */
    u16 saved_cur;      /* +0x76 */
    u8 pad78[0x44];
    UiObj *cursor[2];   /* +0xBC */
    u8 padC4[0x1C];
    UiObj *label[2];    /* +0xE0 */
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
extern void func_8008C088(void);
extern void func_80095130(void);
extern void func_80094E08(void);
extern void func_8005BA2C(void);
extern void func_8008D060(void);
extern void func_8008D2E0(void);
extern void func_8008DD50(void);
extern void func_8008E70C(void);
extern void func_800902EC(void);
extern void func_80090B90(void);
extern void func_800AE30C(void);
extern void func_800506EC(s32 n);
extern void func_8005C2D0(s32 a, s32 x, s32 y, s32 w, s32 id);
extern void func_8009504C(UiObj *o);
extern void func_800953A0(UiObj *o);
extern void func_8008DCB0(UiObj *o);

void func_8008D560(UiObj *o)
{
    UiObj *menu;

    menu = obj_find_by_handle(func_80095130);
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) {
            menu->light = 16;
            menu->update = ui_obj_nop2;
            D_8019C090.cursor[0]->light = 16;
            D_8019C090.cursor[1]->light = 16;
            D_8019C090.label[0]->light = 16;
            D_8019C090.label[1]->light = 16;
            o->update = ui_obj_nop2;
            if (D_8019C090.mode == 0) {
                func_8008D060();
                func_8008DD50();
            } else {
                func_8008D2E0();
                if (D_8019C090.total >= 4) {
                    D_8019C090.rows = 3;
                } else {
                    D_8019C090.rows = D_8019C090.total;
                }
                if (D_8019C090.rows <= 0) {
                    D_801A2309 = 1;
                    D_801A230A = 2;
                    D_801A2314 = func_8008C088;
                    D_801A230B = 10;
                    D_801A230C = 14;
                    D_801A230D = 13;
                    D_801A230E = 6;
                    D_801A2310 = D_800B8C98;
                    func_800AE30C();
                } else {
                    D_8019C090.cur = D_8019C090.saved_cur;
                    if (D_8019C090.cur > D_8019C090.total - D_8019C090.rows) {
                        D_8019C090.cur = D_8019C090.total - D_8019C090.rows;
                    }
                    D_8019C090.top = D_8019C090.saved_top;
                    if (D_8019C090.top > D_8019C090.total - 1) {
                        D_8019C090.top = D_8019C090.total - 1;
                    }
                    func_8008E70C();
                    func_800902EC();
                    func_80090B90();
                }
            }
        } else if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
            SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
            func_800506EC(16);
            obj_find_by_id(15)->kill = 1;
            func_8005C2D0(4, 196, 144, 24, 15);
            obj_find_by_handle(func_80094E08)->update = func_8009504C;
            menu->update = func_800953A0;
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(4, 80, 112, 48, 3);
            o->update = func_8008DCB0;
        }
    }
}
