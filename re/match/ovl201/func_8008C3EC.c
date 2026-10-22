#include "ac1/ac1.h"
/* ovl201 0x8008C3EC - shop controller constructor: reset the shop state
 * (mode, selection, saved cursor/top per category and for the sell list),
 * make the controller object (update 0x8008CCA8, destroy 0x8008DD48),
 * build the shop window frame in VRAM from 2x24 border tiles (MoveImage
 * from x 548..558), then clear D_801A2574. */

typedef struct {
    u8 pad00;
    s8 mode;            /* +0x01 0 = buy list */
    s8 kind;            /* +0x02 selected category */
    s8 first;           /* +0x03 category in the top row */
    u8 pad04[0x56];
    s16 top;            /* +0x5A */
    s16 total;          /* +0x5C */
    u16 sell_top;       /* +0x5E */
    u16 saved_top[10];  /* +0x60 per category */
    s16 cur;            /* +0x74 */
    u16 sell_cur;       /* +0x76 */
    u16 saved_cur[10];  /* +0x78 per category */
} ShopState;
extern ShopState D_8019C090;
extern s32 D_801A2574;
extern void func_8008CCA8(UiObj *o);
extern void func_8008DD48(UiObj *o);
extern void func_8005D71C(void);
extern void func_80095094(void);
extern void func_8005CC48(void);

UiObj *func_8008C3EC(void)
{
    RECT r = { 960, 0, 2, 24 };
    UiObj *o;
    u8 i;

    D_8019C090.mode = 0;
    D_8019C090.top = 0;
    D_8019C090.kind = 0;
    D_8019C090.first = 0;
    D_8019C090.sell_top = 0;
    D_8019C090.sell_cur = 0;
    i = 9;
    do {
        D_8019C090.saved_cur[i] = 0;
        D_8019C090.saved_top[i] = 0;
    } while (--i != 255);
    o = ui_obj_alloc(0);
    o->ctor = func_8008C3EC;
    o->update = func_8008CCA8;
    o->destroy = func_8008DD48;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    func_8005D71C();
    func_80095094();
    r.x = 548;
    r.y = 24;
    MoveImage(&r, 960, 32);
    r.x = 550;
    r.y = 24;
    MoveImage(&r, 962, 32);
    MoveImage(&r, 964, 32);
    MoveImage(&r, 966, 32);
    MoveImage(&r, 968, 32);
    MoveImage(&r, 970, 32);
    MoveImage(&r, 972, 32);
    MoveImage(&r, 974, 32);
    MoveImage(&r, 976, 32);
    MoveImage(&r, 978, 32);
    MoveImage(&r, 980, 32);
    MoveImage(&r, 982, 32);
    MoveImage(&r, 984, 32);
    r.x = 552;
    r.y = 24;
    MoveImage(&r, 986, 32);
    r.x = 554;
    r.y = 24;
    MoveImage(&r, 960, 56);
    r.x = 556;
    r.y = 24;
    MoveImage(&r, 962, 56);
    MoveImage(&r, 964, 56);
    MoveImage(&r, 966, 56);
    MoveImage(&r, 968, 56);
    MoveImage(&r, 970, 56);
    MoveImage(&r, 972, 56);
    MoveImage(&r, 974, 56);
    MoveImage(&r, 976, 56);
    MoveImage(&r, 978, 56);
    MoveImage(&r, 980, 56);
    MoveImage(&r, 982, 56);
    MoveImage(&r, 984, 56);
    r.x = 558;
    r.y = 24;
    MoveImage(&r, 986, 56);
    r.x = 548;
    r.y = 24;
    MoveImage(&r, 960, 80);
    r.x = 550;
    r.y = 24;
    MoveImage(&r, 962, 80);
    MoveImage(&r, 964, 80);
    MoveImage(&r, 966, 80);
    MoveImage(&r, 968, 80);
    MoveImage(&r, 970, 80);
    MoveImage(&r, 972, 80);
    MoveImage(&r, 974, 80);
    MoveImage(&r, 976, 80);
    MoveImage(&r, 978, 80);
    MoveImage(&r, 980, 80);
    MoveImage(&r, 982, 80);
    MoveImage(&r, 984, 80);
    MoveImage(&r, 986, 80);
    MoveImage(&r, 988, 80);
    MoveImage(&r, 990, 80);
    MoveImage(&r, 992, 80);
    r.x = 552;
    r.y = 24;
    MoveImage(&r, 994, 80);
    r.x = 554;
    r.y = 24;
    MoveImage(&r, 960, 104);
    r.x = 556;
    r.y = 24;
    MoveImage(&r, 962, 104);
    MoveImage(&r, 964, 104);
    MoveImage(&r, 966, 104);
    MoveImage(&r, 968, 104);
    MoveImage(&r, 970, 104);
    MoveImage(&r, 972, 104);
    MoveImage(&r, 974, 104);
    MoveImage(&r, 976, 104);
    MoveImage(&r, 978, 104);
    MoveImage(&r, 980, 104);
    MoveImage(&r, 982, 104);
    MoveImage(&r, 984, 104);
    MoveImage(&r, 986, 104);
    MoveImage(&r, 988, 104);
    MoveImage(&r, 990, 104);
    MoveImage(&r, 992, 104);
    r.x = 558;
    r.y = 24;
    MoveImage(&r, 994, 104);
    r.x = 548;
    r.y = 24;
    MoveImage(&r, 960, 128);
    r.x = 550;
    r.y = 24;
    MoveImage(&r, 962, 128);
    MoveImage(&r, 964, 128);
    MoveImage(&r, 966, 128);
    MoveImage(&r, 968, 128);
    MoveImage(&r, 970, 128);
    MoveImage(&r, 972, 128);
    MoveImage(&r, 974, 128);
    MoveImage(&r, 976, 128);
    MoveImage(&r, 978, 128);
    MoveImage(&r, 980, 128);
    MoveImage(&r, 982, 128);
    MoveImage(&r, 984, 128);
    MoveImage(&r, 986, 128);
    MoveImage(&r, 988, 128);
    MoveImage(&r, 990, 128);
    MoveImage(&r, 992, 128);
    MoveImage(&r, 994, 128);
    MoveImage(&r, 996, 128);
    MoveImage(&r, 998, 128);
    MoveImage(&r, 1000, 128);
    r.x = 552;
    r.y = 24;
    MoveImage(&r, 1002, 128);
    r.x = 554;
    r.y = 24;
    MoveImage(&r, 960, 152);
    r.x = 556;
    r.y = 24;
    MoveImage(&r, 962, 152);
    MoveImage(&r, 964, 152);
    MoveImage(&r, 966, 152);
    MoveImage(&r, 968, 152);
    MoveImage(&r, 970, 152);
    MoveImage(&r, 972, 152);
    MoveImage(&r, 974, 152);
    MoveImage(&r, 976, 152);
    MoveImage(&r, 978, 152);
    MoveImage(&r, 980, 152);
    MoveImage(&r, 982, 152);
    MoveImage(&r, 984, 152);
    MoveImage(&r, 986, 152);
    MoveImage(&r, 988, 152);
    MoveImage(&r, 990, 152);
    MoveImage(&r, 992, 152);
    MoveImage(&r, 994, 152);
    MoveImage(&r, 996, 152);
    MoveImage(&r, 998, 152);
    MoveImage(&r, 1000, 152);
    r.x = 558;
    r.y = 24;
    MoveImage(&r, 1002, 152);
    DrawSync(0);
    func_8005CC48();
    D_801A2574 = 0;
    return o;
}
