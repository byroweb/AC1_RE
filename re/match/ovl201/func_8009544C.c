#include "ac1/ac1.h"
/* ovl201 0x8009544C - ten-entry list cursor: up/down move the selection
 * D_8019C092 (0..9) and the cursor y (0..72 by 24); at the top/bottom row
 * the visible rows are scrolled in VRAM by 16 (MoveImage), the new row is
 * drawn (func_8008DE30) and the first visible row D_8019C093 moves.  Held
 * up/down feeds the key-repeat counters D_800B8B70/D_800B8B74. */
extern s8 D_8019C092;
extern u8 D_8019C093;
extern RECT D_8004CE40;
extern s32 D_800B8B70;
extern s32 D_800B8B74;
extern s16 D_801A2318;
extern u32 D_801A2564;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s32 D_801A2574;
extern int MoveImage(RECT *r, int x, int y);
extern void func_8008DE30(u8 row, s32 slot);

void func_8009544C(UiObj *o)
{
    s8 *sel = &D_8019C092;
    s32 old = *sel;
    s32 y;
    s32 i;
    RECT r;

    if ((D_801A2568 & 0x1000) && !(D_801A256C & 0x1000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (--*sel < 0) {
            *sel = 0;
        }
        y = o->pos.vy;
        if (y > 0) {
            y -= 24;
            goto store;
        }
        r = D_8004CE40;
        if (old != *sel) {
            i = 3;
        up_loop:
            {
                s32 j = i - 1;
                r.y = j * 16 + 0xB0;
                MoveImage(&r, r.x, r.y + 16);
                i = j;
            }
            if (i != 0) {
                goto up_loop;
            }
            func_8008DE30(D_8019C092, 0);
            D_8019C093--;
        }
    } else if ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000)) {
        s8 *p;

        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        p = &D_8019C092;
        if (++*p >= 10) {
            *p = 9;
        }
        if (o->pos.vy >= 72) {
            r = D_8004CE40;
            if (old != *p) {
                for (i = 1; i < 4; i++) {
                    r.y = i * 16 + 0xB0;
                    MoveImage(&r, r.x, r.y - 16);
                }
                func_8008DE30(D_8019C092, 3);
                D_8019C093++;
            }
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
