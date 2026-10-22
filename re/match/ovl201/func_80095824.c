#include "ac1/ac1.h"
/* ovl201 0x80095824 - part-shop list cursor (built by func_80095784):
 * up/down move the selection PartList.sel over count entries (wrapping
 * when the list fits on screen), refresh the info panes (func_80090668,
 * func_800908F8) and, at the top/bottom row of a long list, scroll the two
 * VRAM columns by 16 px, shift the row label lights and draw the new row
 * (func_8008F344 + func_8008F274 for shop lists, func_8008F2D0 otherwise). */
typedef struct {
    u8 pad00;
    s8 mode;            /* +0x01 0x8019C091 */
    s8 kind;            /* +0x02 0x8019C092 */
    u8 unk03;
    s8 rows;            /* +0x04 0x8019C094 visible rows */
    u8 pad05[0x53];
    s16 count;          /* +0x58 0x8019C0E8 */
    s16 sel;            /* +0x5A 0x8019C0EA */
    u8 pad5c[0x18];
    u16 first;          /* +0x74 0x8019C104 */
    u8 pad76[0x20];
    s16 base[49];       /* +0x96 0x8019C126 first entry of each kind */
    UiObj *lbl[6];      /* +0xF8 0x8019C188 name labels 0..2, info labels 3..5 */
    u8 pad110[0xC];
    u8 owner[0xA0];     /* +0x11C 0x8019C1AC */
    u8 id[0x100];       /* +0x1BC 0x8019C24C */
} PartList;

extern PartList D_8019C090;
extern RECT D_8004CE48;
extern RECT D_8004CE50;
extern s32 D_800B8B70;
extern s32 D_800B8B74;
extern s16 D_801A2318;
extern u32 D_801A2564;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s32 D_801A2574;
extern int MoveImage(RECT *r, int x, int y);
extern s32 func_800517C8(void);
extern void func_8008F274(u8 kind, u8 id, u8 row);
extern void func_8008F2D0(u8 owner, u8 id, u8 row);
extern void func_8008F344(u8 kind, u8 id, u8 row);
extern void func_80090668(void);
extern void func_800908F8(void);
extern void func_80090B90(UiObj *o);

void func_80095824(UiObj *o)
{
    s32 old;
    s32 i;
    s16 idx;
    RECT r1;
    RECT r2;
    UiObj *p;

    old = D_8019C090.sel;
    if ((D_801A2568 & 0x1000) && !(D_801A256C & 0x1000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (--D_8019C090.sel < 0) {
            if (D_8019C090.count < 4) {
                D_8019C090.sel += D_8019C090.count;
            } else {
                D_8019C090.sel = 0;
            }
        }
        if (old != D_8019C090.sel) {
            func_80090668();
            func_800908F8();
        }
        if (o->pos.vy <= 0) {
            r1 = D_8004CE48;
            r2 = D_8004CE50;
            if (D_8019C090.count < 4) {
                o->pos.vy = (D_8019C090.count - 1) * 24;
            } else if (old != D_8019C090.sel) {
                i = D_8019C090.rows;
                while (--i != 0) {
                    r1.y = (i - 1) * 16;
                    MoveImage(&r1, r1.x, r1.y + 16);
                    r2.y = (i - 1) * 16 + 128;
                    MoveImage(&r2, r2.x, r2.y + 16);
                    D_8019C090.lbl[i]->light = D_8019C090.lbl[i - 1]->light;
                    D_8019C090.lbl[i + 3]->light = D_8019C090.lbl[i + 2]->light;
                }
                if (D_8019C090.mode == 0) {
                    idx = D_8019C090.sel + D_8019C090.base[D_8019C090.kind];
                    func_8008F344(D_8019C090.kind, D_8019C090.id[idx], 0);
                    func_8008F274(D_8019C090.kind, D_8019C090.id[idx], 0);
                } else {
                    func_8008F344(D_8019C090.owner[D_8019C090.sel], D_8019C090.id[D_8019C090.sel], 0);
                    func_8008F2D0(D_8019C090.owner[D_8019C090.sel], D_8019C090.id[D_8019C090.sel], 0);
                }
                D_8019C090.first--;
            }
        } else {
            o->pos.vy -= 24;
        }
    } else if ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (++D_8019C090.sel > D_8019C090.count - 1) {
            if (D_8019C090.count < 4) {
                D_8019C090.sel -= D_8019C090.count;
            } else {
                D_8019C090.sel = D_8019C090.count - 1;
            }
        }
        if (old != D_8019C090.sel) {
            func_80090668();
            func_800908F8();
        }
        if (o->pos.vy >= (D_8019C090.rows - 1) * 24) {
            r1 = D_8004CE48;
            r2 = D_8004CE50;
            if (D_8019C090.count < 4) {
                o->pos.vy = 0;
            } else if (old != D_8019C090.sel) {
                for (i = 1; i < D_8019C090.rows; i++) {
                    r1.y = i * 16;
                    MoveImage(&r1, r1.x, r1.y - 16);
                    r2.y = i * 16 + 128;
                    MoveImage(&r2, r2.x, r2.y - 16);
                    D_8019C090.lbl[i - 1]->light = D_8019C090.lbl[i]->light;
                    D_8019C090.lbl[i + 2]->light = D_8019C090.lbl[i + 3]->light;
                }
                if (D_8019C090.mode == 0) {
                    idx = D_8019C090.sel + D_8019C090.base[D_8019C090.kind];
                    func_8008F344(D_8019C090.kind, D_8019C090.id[idx], D_8019C090.rows - 1);
                    func_8008F274(D_8019C090.kind, D_8019C090.id[idx], D_8019C090.rows - 1);
                } else {
                    func_8008F344(D_8019C090.owner[D_8019C090.sel], D_8019C090.id[D_8019C090.sel], D_8019C090.rows - 1);
                    func_8008F2D0(D_8019C090.owner[D_8019C090.sel], D_8019C090.id[D_8019C090.sel], D_8019C090.rows - 1);
                }
                D_8019C090.first++;
            }
        } else {
            o->pos.vy += 24;
        }
    }
    if ((s16)func_800517C8() == 0) {
        p = obj_find_by_handle(func_80090B90);
        p->visible = 1;
        p->attached = 1;
    }
    if (!(D_801A2564 & 0xF000)) {
        D_800B8B70 = 0;
    } else {
        D_800B8B70 += 1 << D_801A2574;
    }
    if (D_800B8B70 >= 20) {
        D_800B8B74 += 1 << D_801A2574;
        if (D_800B8B74 >= 2) {
            D_800B8B74 = 0;
            D_801A2564 = 0;
        }
    }
}
