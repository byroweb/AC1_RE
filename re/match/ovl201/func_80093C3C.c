#include "ac1/ac1.h"
/* ovl201 0x80093C3C - once no func_8005BA2C object is left, show the
 * description of the selected shop part: copy its two-byte text line
 * (78-byte rows at 0x8019C360, ended by 13) after an "@0" prefix and make a
 * text label of it; then wait in func_80093E3C. */

typedef struct {
    u8 pad00;
    s8 mode;            /* +0x01 0 = buy list */
    s8 kind;            /* +0x02 selected category */
    u8 pad03[4];
    u8 text[0x53];      /* +0x07 label text */
    u16 top;            /* +0x5A */
    u8 pad5c[0x3A];
    u16 start[10];      /* +0x96 */
    u8 padAA[0x72];
    u8 kinds[0xA0];     /* +0x11C */
    s8 ids[0xA0];       /* +0x1BC */
} ShopState;
extern ShopState D_8019C090;
extern u8 D_8019C360[][78];
extern void func_8005BA2C(void);
extern void func_800506EC(s32 n);
extern UiObj *func_8005CB94(u8 *text, s32 x, s32 y, s32 z);
extern void func_80093E3C(UiObj *o);

void func_80093C3C(UiObj *o)
{
    u16 first[10] = { 0, 10, 13, 29, 61, 68, 76, 87, 93, 127 };
    u8 kind;
    s16 id;
    u8 *p;
    u8 *q;
    u8 c;
    u8 *t;

    t = D_8019C090.text;
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        if (D_8019C090.mode == 0) {
            id = D_8019C090.ids[(s16)(D_8019C090.top + D_8019C090.start[D_8019C090.kind])];
            kind = D_8019C090.kind;
        } else {
            id = D_8019C090.ids[(s16)D_8019C090.top];
            kind = D_8019C090.kinds[(s16)D_8019C090.top];
        }
        func_800506EC(18);
        t[0] = '@';
        t[1] = '0';
        p = D_8019C360[(u16)(first[kind] + id)];
        c = *p;
        q = t + 2;
        while (c != 13) {
            p++;
            *q++ = c;
            *q++ = *p++;
            *q = 0;
            c = *p;
        }
        q[0] = '>';
        q[1] = 0;
        func_8005CB94(t, 8, 4, 18);
        o->update = func_80093E3C;
    }
}
