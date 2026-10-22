#include "ac1/ac1.h"
/* ovl201 0x800951B0 - two-row cursor update (built by func_80095130): up/down
 * edges move the row index D_8019C091 (0..1, wrapping) and the cursor y by
 * 24; holding up/down feeds the key-repeat counters D_800B8B70/D_800B8B74. */
extern s8 D_8019C091;
extern s32 D_800B8B70;
extern s32 D_800B8B74;
extern s16 D_801A2318;
extern u32 D_801A2564;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s32 D_801A2574;

void func_800951B0(UiObj *o)
{
    s8 *sel;
    s32 y;

    if ((D_801A2568 & 0x1000) && !(D_801A256C & 0x1000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        sel = &D_8019C091;
        if (--*sel < 0) {
            *sel += 2;
        }
        {
            y = o->pos.vy;
            if (y > 0) {
                y -= 24;
            } else {
                y = 24;
            }
            goto store;
        }
    } else if ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        sel = &D_8019C091;
        if (++*sel >= 2) {
            *sel -= 2;
        }
        if (o->pos.vy >= 24) {
            o->pos.vy = 0;
        } else {
            y = o->pos.vy + 24;
        store:
            o->pos.vy = y;
        }
    }
    if (!(D_801A2564 & 0x5000)) {
        D_800B8B70 = 0;
    } else {
        D_800B8B70 += 1 << D_801A2574;
    }
    if (D_800B8B70 >= 20) {
        D_800B8B74 += 1 << D_801A2574;
        if (D_800B8B74 >= 5) {
            D_800B8B74 = 0;
            D_801A2564 = 0;
        }
    }
}
