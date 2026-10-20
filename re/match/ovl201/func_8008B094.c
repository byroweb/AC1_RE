#include "ac1/ac1.h"
/* ovl201 func_8008B094 - up/down handling of a scrolling name list.  A new
 * press of up (0x1000) / down (0x4000) plays the cursor sound and moves the
 * cursor (wrapping when the list fits on screen).  While the highlight can
 * still move it slides 24 lines; at the edge of a list longer than five
 * rows the row names shift one place (func_8008B048) and the drawn rows
 * are moved 16 lines in VRAM, then the row texts are rebuilt.  A held pad
 * (0x5000) for 20 frames, five times, clears D_801A2564. */

extern u32 D_801A2564;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s16 D_801A2318;
extern UiListState D_801F37C8;
extern u8 D_801F37D4[];
extern UiNameList *D_801F5094;
extern s32 D_800B8A6C;
extern s32 D_800B8A70;
extern void func_8008B048(char *dst, char *src);
extern void func_80087F58(u8 id, u8 row);
extern void func_80087EB8(u8 id, u8 row);
extern void func_80087950(void);

void func_8008B094(UiObj *o)
{
    s32 prev;
    s32 i;

    prev = D_801F37C8.cursor;
    if ((D_801A2568 & 0x1000) && !(D_801A256C & 0x1000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (--D_801F37C8.cursor < 0) {
            if (D_801F5094->count < 6) {
                D_801F37C8.cursor += D_801F5094->count;
            } else {
                D_801F37C8.cursor = 0;
            }
        }
        if (o->pos.vy <= 0) {
            RECT rect = { 960, 176, 48, 16 };

            if (D_801F5094->count < 6) {
                o->pos.vy = (D_801F5094->count - 1) * 24;
            } else if (prev != D_801F37C8.cursor) {
                i = D_801F5094->rows;
                for (i--; i != 0; i--) {
                    func_8008B048(D_801F5094->name[i + 1], D_801F5094->name[i]);
                    rect.y = (i - 1) * 16 + 176;
                    MoveImage(&rect, rect.x, rect.y + 16);
                }
                func_80087F58(D_801F37D4[D_801F37C8.cursor], 0);
                func_80087EB8(D_801F37D4[D_801F37C8.cursor], 0);
                D_801F37C8.scroll--;
                func_80087950();
            }
        } else {
            o->pos.vy -= 24;
        }
    } else if ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        D_801F37C8.cursor++;
        if (D_801F37C8.cursor > D_801F5094->count - 1) {
            if (D_801F5094->count < 6) {
                D_801F37C8.cursor -= D_801F5094->count;
            } else {
                D_801F37C8.cursor = D_801F5094->count - 1;
            }
        }
        if (o->pos.vy >= (D_801F5094->rows - 1) * 24) {
            RECT rect = { 960, 176, 48, 16 };

            if (D_801F5094->count < 6) {
                o->pos.vy = 0;
            } else if (prev != D_801F37C8.cursor) {
                for (i = 1; i < D_801F5094->rows; i++) {
                    func_8008B048(D_801F5094->name[i], D_801F5094->name[i + 1]);
                    rect.y = i * 16 + 176;
                    MoveImage(&rect, rect.x, rect.y - 16);
                }
                func_80087F58(D_801F37D4[D_801F37C8.cursor], D_801F5094->rows - 1);
                func_80087EB8(D_801F37D4[D_801F37C8.cursor], D_801F5094->rows - 1);
                D_801F37C8.scroll++;
                func_80087950();
            }
        } else {
            o->pos.vy += 24;
        }
    }
    if (!(D_801A2564 & 0x5000)) {
        D_800B8A6C = 0;
    } else {
        D_800B8A6C++;
    }
    if (D_800B8A6C >= 20) {
        if (++D_800B8A70 >= 5) {
            D_800B8A70 = 0;
            D_801A2564 = 0;
        }
    }
}
