#include "ac1/ac1.h"
/* ovl201 0x8008F3B4 - part list setup (once no func_8005BA2C object is
 * left): make the two label columns of the visible rows, clamp the cursor
 * and top row to the list length, draw every visible row (name via
 * func_8008F274 / func_8008F2D0, price via func_8008F344) and make the row
 * cursors; then run func_8008FA94. */

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
    u8 pad5e[0x16];
    s16 cur;            /* +0x74 first visible row */
    u8 pad76[0x16];
    s8 count[10];       /* +0x8C */
    u16 start[10];      /* +0x96 */
    u8 padAA[0x2A];
    UiObj *cursor[3];   /* +0xD4 */
    u8 padE0[0x18];
    UiObj *name_label[3];       /* +0xF8 */
    UiObj *price_label[3];      /* +0x104 */
    u8 pad110[0xC];
    u8 kinds[0xA0];     /* +0x11C */
    u8 ids[0xA0];       /* +0x1BC */
} ShopState;
extern ShopState D_8019C090;
extern u8 D_800B8BB4[];
extern u8 D_800B8C2C[][12];
extern u8 D_800B8C50[][12];
extern void func_8005BA2C(void);
extern void func_80095784(void);
extern UiObj *func_8005CAE0(void *res, s32 x, s32 y, s32 z);
extern void func_8008F274(u8 kind, u8 id, u8 row);
extern void func_8008F2D0(u8 kind, u8 id, u8 row);
extern void func_8008F344(u8 kind, u8 id, u8 row);
extern void func_8008FA94(UiObj *o);

void func_8008F3B4(UiObj *o)
{
    s32 i;
    s16 j;
    s16 k;
    s32 kind;
    s32 unused[4];      /* 16 frame bytes retail reserves but never touches */

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        func_80095784();
        i = D_8019C090.rows;
        while (--i != -1) {
            D_8019C090.name_label[i] = func_8005CAE0(D_800B8C2C[i], 8, (s16)(i * 24 + 5), 10);
            D_8019C090.name_label[i]->light = 20;
        }
        DrawSync(0);
        i = D_8019C090.rows;
        while (--i != -1) {
            D_8019C090.price_label[i] = func_8005CAE0(D_800B8C50[i], 112, (s16)(i * 24 + 5), 10);
            D_8019C090.price_label[i]->light = 20;
        }
        if (D_8019C090.mode == 0) {
            D_8019C090.n = D_8019C090.count[D_8019C090.kind];
            if (D_8019C090.n != 0) {
                if (D_8019C090.top >= D_8019C090.n) {
                    D_8019C090.top = D_8019C090.n - 1;
                    D_8019C090.cur = D_8019C090.n - 3;
                }
            } else {
                D_8019C090.top = 0;
            }
            i = D_8019C090.rows;
        while (--i != -1) {
                k = (D_8019C090.cur + i) % D_8019C090.n;
                kind = D_8019C090.kind;
                func_8008F274(kind, D_8019C090.ids[(s16)(D_8019C090.start[kind] + k)], i);
            }
            i = D_8019C090.rows;
        while (--i != -1) {
                k = (D_8019C090.cur + i) % D_8019C090.n;
                kind = D_8019C090.kind;
                func_8008F344(kind, D_8019C090.ids[(s16)(D_8019C090.start[kind] + k)], i);
            }
        } else {
            D_8019C090.n = D_8019C090.count[0] + D_8019C090.count[1] + D_8019C090.count[2] +
                D_8019C090.count[3] + D_8019C090.count[4] + D_8019C090.count[5] +
                D_8019C090.count[6] + D_8019C090.count[7] + D_8019C090.count[8] +
                D_8019C090.count[9];
            if (D_8019C090.total != 0) {
                if (D_8019C090.top >= D_8019C090.total) {
                    D_8019C090.top = D_8019C090.total - 1;
                    D_8019C090.cur = D_8019C090.total - 3;
                }
            } else {
                D_8019C090.top = 0;
            }
            i = D_8019C090.rows;
        while (--i != -1) {
                j = (D_8019C090.cur + i) % D_8019C090.total;
                func_8008F2D0(D_8019C090.kinds[j], D_8019C090.ids[j], i);
            }
            i = D_8019C090.rows;
        while (--i != -1) {
                j = (D_8019C090.cur + i) % D_8019C090.total;
                func_8008F344(D_8019C090.kinds[j], D_8019C090.ids[j], i);
            }
        }
        i = D_8019C090.rows;
        while (--i != -1) {
            D_8019C090.cursor[i] = func_8005CAE0(D_800B8BB4, 0, (s16)(i * 24), 10);
        }
        o->update = func_8008FA94;
    }
}
