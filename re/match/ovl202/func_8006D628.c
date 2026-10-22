#include "ac1/ac1.h"

extern AcEntity *D_800908EC;
extern SVECTOR **D_800908F0;
extern WorldStruct *D_800908F4;
extern s32 D_800908F8;
extern s32 D_800908FC;
extern s32 D_80090900;
extern s32 D_80090908;
extern s32 D_8009090C;
extern s32 D_80090910;
extern SVECTOR D_80090914;
extern AcEntity *D_8009091C;
extern s32 func_800126D8(SVECTOR *c, s32 h, s32 hh, SVECTOR *p);

/* Next hit of the area query set up by func_8006D8F8: entities, then the
 * point list, then world structs; *dist gets the distance. */
void *func_8006D628(s32 *dist) {
    SVECTOR v;
    AcEntity *e;
    SVECTOR *pt;
    WorldStruct *w;
    s32 d;

    while (D_800908F8 != 0) {
        if (D_8009091C != D_800908EC && D_800908EC->def != 0) {
            e = D_800908EC;
            v.vx = e->pos.x;
            v.vy = e->pos.y - (*(u16 *)((u8 *)e + 0x6C) >> 1);
            v.vz = e->pos.z;
            d = func_800126D8(&D_80090914, D_80090908, D_80090910, &v);
            if (d >= 0 && d >= D_8009090C) {
                *dist = d;
                D_800908F8--;
                return D_800908EC++;
            }
        }
        D_800908EC++;
        D_800908F8--;
    }
    while (D_800908FC != 0) {
        pt = *D_800908F0++;
        d = func_800126D8(&D_80090914, D_80090908, D_80090910, pt);
        if (d >= 0 && d >= D_8009090C) {
            *dist = d;
            D_800908FC--;
            return pt;
        }
        D_800908FC--;
    }
    while (D_80090900 != 0) {
        w = D_800908F4;
        if (*(s16 *)&w->unk00[6] != -1 && (*(u32 *)w->unk30 & 0x20000)) {
            d = func_800126D8(&D_80090914, D_80090908, D_80090910, (SVECTOR *)w->org);
            if (d >= 0 && d >= D_8009090C) {
                *dist = d;
                D_80090900--;
                return D_800908F4++;
            }
        }
        D_800908F4++;
        D_80090900--;
    }
    return 0;
}
