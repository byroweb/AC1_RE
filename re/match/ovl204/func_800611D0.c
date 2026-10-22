#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x3C];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
} Ent;

typedef struct {
    u8 type;            /* +0x00 */
    u8 pad1[0xB];
    u8 flags;           /* +0x0C */
    u8 padD[0x3C - 0xD];
} WpnRec;

typedef struct {
    u8 b[0x30];
} Item;

typedef struct {
    u8 unk0[0x18];
    u8 cur;             /* +0x18 */
    u8 unk19[0x80 - 0x19];
    s32 unk80;          /* +0x80 */
    Item items[8];      /* +0x84 */
    u8 unk204[0x27A - 0x204];
    u8 unk27A;          /* +0x27A */
    u8 unk27B[0x2D8 - 0x27B];
    u8 unk2D8;          /* +0x2D8 */
    u8 unk2D9;
    WpnRec rec[1];      /* +0x2DA */
    u8 pad[0x394 - 0x2DA - 0x3C];
    Ent *ent;           /* +0x394 */
} Player;

typedef struct {
    s32 on;             /* +0x000 */
    u8 pad4[0xE98 - 4];
    MATRIX m;           /* +0xE98 */
} Scene;

extern Player *D_80039D18;
extern Scene D_801B4104;
extern void func_800513B4(void);
extern void func_80050888(void);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void func_80051D14(Item *it);
extern void func_80051878(WpnRec *r, Item *it);
extern void func_80052020(Item *it);
extern void func_80052244(s32 a);
extern void func_80052368(Item *it);

void func_800611D0(void)
{
    WpnRec *r;
    Item *it;
    s32 i;

    func_800513B4();
    func_80050888();
    if (D_801B4104.on == 0 || D_80039D18->unk2D8 == 0 || D_80039D18->ent->dir == 32) {
        return;
    }
    SetRotMatrix(&D_801B4104.m);
    SetTransMatrix((MATRIX *)((u8 *)&D_801B4104.m + 12));
    r = &D_80039D18->rec[D_80039D18->cur];
    switch (r->type) {
    case 0:
        if (!(r->flags & 4)) {
            func_80051D14(D_80039D18->items);
        }
        func_80051878(r, D_80039D18->items);
        break;
    case 1:
        i = 8;
        it = D_80039D18->items;
        do {
            func_80052020(it);
            i--;
            it++;
        } while (i != 0);
        func_80052244(D_80039D18->unk80);
        func_80051878(r, 0);
        break;
    case 2:
        if (D_80039D18->ent->dir == 34 || D_80039D18->unk27A == 1 || D_80039D18->unk27A == 3
            || (D_80039D18->unk27A == 2 && D_80039D18->ent->kind != 192)) {
            func_80052368(D_80039D18->items);
            func_80051878(r, D_80039D18->items);
        }
        break;
    }
}
