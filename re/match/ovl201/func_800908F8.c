#include "ac1/ac1.h"
/* ovl201 0x800908F8 - load the texture/palette set of the selected shop
 * part through func_80051450(base, index, extra): the base per category
 * (table 0x8004CD5C, kind 9 entries 16.. use 14); head/core/arms/legs add
 * the paint scheme D_80039CA2 times a per-category stride (0x8004CD70). */

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
extern u8 D_80039CA2;
extern void func_80051450(u16 base, u16 idx, s32 extra);

void func_800908F8(void)
{
    u16 base[10] = { 10, 7, 9, 8, 18, 16, 17, 11, 12, 15 };
    s16 stride[10] = { 10, 3, 16, 32, 7, 8, 11, 6, 34, 20 };
    u16 i;
    u8 k;
    u8 m;
    s32 extra;

    if (D_8019C090.mode == 0) {
        i = D_8019C090.top + D_8019C090.start[D_8019C090.kind];
        k = D_8019C090.kind;
        if (k == 0 || k == 1 || k == 2 || k == 3) {
            extra = D_80039CA2 * stride[k];
        } else {
            extra = 0;
        }
        if (D_8019C090.kind == 9 && D_8019C090.ids[(s16)i] >= 16) {
            func_80051450(14, D_8019C090.ids[(s16)i] - 16, extra);
        } else {
            func_80051450(base[D_8019C090.kind], D_8019C090.ids[(s16)i] + extra, extra);
        }
    } else {
        m = D_8019C090.kinds[(s16)D_8019C090.top];
        if (m == 0 || m == 1 || m == 2 || m == 3) {
            extra = D_80039CA2 * stride[m];
        } else {
            extra = 0;
        }
        if (D_8019C090.kinds[(s16)D_8019C090.top] == 9 && D_8019C090.ids[(s16)D_8019C090.top] >= 16) {
            func_80051450(14, D_8019C090.ids[(s16)D_8019C090.top] - 16, extra);
        } else {
            func_80051450(base[D_8019C090.kinds[(s16)D_8019C090.top]], D_8019C090.ids[(s16)D_8019C090.top] + extra, extra);
        }
    }
}
