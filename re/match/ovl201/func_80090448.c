#include "ac1/ac1.h"
/* ovl201 0x80090448 - place and scale the part viewer for the selected shop
 * row: model group by category (table 0x8004CD3C; kind 9 entries 16.. use
 * group 3), per-model scale and height from the group's 16-byte records. */

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
typedef struct {
    s16 unk0;
    s16 y;              /* +0x02 */
    s16 z;              /* +0x04 */
    s16 unk6;
    s32 unk8;
    s32 scale;          /* +0x0C */
} ModelPlace;
typedef struct {
    ModelPlace *place;
    u8 pad04[12];
} ModelGroup;
extern ShopState D_8019C090;
extern ModelGroup D_800BCAC0[];
extern void func_80090B90(void);

void func_80090448(void)
{
    UiObj *v = obj_find_by_handle(func_80090B90);
    u8 group[10] = { 4, 0, 2, 1, 10, 8, 9, 5, 6, 7 };
    s16 i;
    u8 g;
    u8 idx;

    if (D_8019C090.mode == 0) {
        i = D_8019C090.top + D_8019C090.start[D_8019C090.kind];
        if (D_8019C090.kind == 9 && D_8019C090.ids[i] >= 16) {
            g = 3;
            idx = D_8019C090.ids[i] - 16;
        } else {
            g = group[D_8019C090.kind];
            idx = D_8019C090.ids[i];
        }
    } else {
        if (D_8019C090.kinds[(s16)D_8019C090.top] == 9 && D_8019C090.ids[(s16)D_8019C090.top] >= 16) {
            g = 3;
            idx = D_8019C090.ids[(s16)D_8019C090.top] - 16;
        } else {
            g = group[D_8019C090.kinds[(s16)D_8019C090.top]];
            idx = D_8019C090.ids[(s16)D_8019C090.top];
        }
    }
    v->scale.vx = D_800BCAC0[g].place[idx].scale;
    v->scale.vy = D_800BCAC0[g].place[idx].scale;
    v->scale.vz = D_800BCAC0[g].place[idx].scale;
    v->pos.vx = 0;
    v->pos.vy = D_800BCAC0[g].place[idx].y - (s16)(D_800BCAC0[g].place[idx].z * 10 / 256 - 10);
    v->pos.vz = D_800BCAC0[g].place[idx].z;
}
