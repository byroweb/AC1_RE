#include "ac1/ac1.h"
/* ovl201 0x80090668 - show the 3D model of the selected shop part: wait for
 * the loader (func_800517C8), load model `first[kind] + index` (kind 9
 * entries 16.. come from model 104 on), hide the viewer and refresh it. */

typedef struct {
    u8 pad00;
    s8 mode;            /* +0x01 0 = buy list */
    s8 kind;            /* +0x02 selected category */
    u8 pad03[0x57];
    u16 top;            /* +0x5A */
    u8 pad5c[0x3A];
    u16 start[10];      /* +0x96 */
    u8 padAA[0x72];
    s8 kinds[0xA0];     /* +0x11C */
    s8 ids[0xA0];       /* +0x1BC */
} ShopState;
extern ShopState D_8019C090;
extern void func_80090B90(void);
extern s16 func_800517C8(void);
extern void func_80051628(void);
extern void func_80050AF4(s32 a, s16 model);
extern void func_80090448(void);

void func_80090668(UiObj *o)
{
    UiObj *v = obj_find_by_handle(func_80090B90);
    s16 model[10] = { 54, 3, 38, 6, 143, 124, 132, 64, 70, 108 };
    s16 i;

    if (func_800517C8()) {
        func_80051628();
    }
    while (func_800517C8()) {
    }
    if (D_8019C090.mode == 0) {
        i = D_8019C090.top + D_8019C090.start[D_8019C090.kind];
        if (D_8019C090.kind == 9 && D_8019C090.ids[i] >= 16) {
            model[9] = 104;
            func_80050AF4(29, model[D_8019C090.kind] + D_8019C090.ids[i] - 16);
        } else {
            func_80050AF4(29, model[D_8019C090.kind] + D_8019C090.ids[i]);
        }
    } else {
        if (D_8019C090.kinds[(s16)D_8019C090.top] == 9 && D_8019C090.ids[(s16)D_8019C090.top] >= 16) {
            model[9] = 104;
            func_80050AF4(29, model[D_8019C090.kinds[(s16)D_8019C090.top]] + D_8019C090.ids[(s16)D_8019C090.top] - 16);
        } else {
            func_80050AF4(29, model[D_8019C090.kinds[(s16)D_8019C090.top]] + D_8019C090.ids[(s16)D_8019C090.top]);
        }
    }
    v->visible = 0;
    v->attached = 0;
    func_80090448();
    if (D_8019C090.mode == 0) {
        if (D_8019C090.kind == 7) {
            *(s16 *)&v->res = 1;
        } else {
            *(s16 *)&v->res = 0;
        }
    } else {
        if (D_8019C090.kinds[(s16)D_8019C090.top] == 7) {
            *(s16 *)&v->res = 1;
        } else {
            *(s16 *)&v->res = 0;
        }
    }
}
