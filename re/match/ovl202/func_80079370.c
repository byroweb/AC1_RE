/* NOT MATCHED: first diff 0x0: pad struct D_80040FA8 addressed via base reg in retail (lui/addiu then lhu 670/666, lw 0) but direct lui/lhu here; body structure matches, 2 attempts */
#include "ac1_types.h"
typedef struct {
    u32 cur;        /* +0x000 */
    u32 prev;       /* +0x004 */
    u8 pad[0x29A - 8];
    u16 maskR;      /* +0x29A */
    u8 pad2[2];
    u16 maskL;      /* +0x29E */
} Func80079370Pad;
typedef struct {
    s16 pos;        /* +0x0 */
    s16 vel;        /* +0x2 */
    s16 timer;      /* +0x4 */
} Func80079370Scroll;
extern Func80079370Pad D_80040FA8;
extern Func80079370Scroll D_800411C0;
void func_80079370(void)
{
    u32 raw;
    u32 mask;
    u32 cur;
    s16 vel;
    raw = D_80040FA8.cur;
    mask = D_80040FA8.maskL | D_80040FA8.maskR;
    cur = raw & mask;
    if (cur != (D_80040FA8.prev & mask)) {
        D_800411C0.timer = 0;
    }
    if (D_800411C0.timer == 0) {
        if (cur == mask) {
            D_800411C0.vel = 0;
            D_800411C0.pos = 0;
            D_800411C0.timer = 10;
        } else {
            if (D_80040FA8.maskL & raw) {
                D_800411C0.vel += 8;
                if (D_800411C0.vel > 48) {
                    D_800411C0.vel = 48;
                }
            } else if (D_80040FA8.maskR & raw) {
                D_800411C0.vel -= 8;
                if (D_800411C0.vel < -48) {
                    D_800411C0.vel = -48;
                }
            } else {
                vel = D_800411C0.vel;
                if (vel > 0) {
                    D_800411C0.vel = vel - 16;
                    if (D_800411C0.vel < 0) {
                        D_800411C0.vel = 0;
                    }
                } else if (vel < 0) {
                    D_800411C0.vel = vel + 16;
                    if (D_800411C0.vel > 0) {
                        D_800411C0.vel = 0;
                    }
                }
            }
            D_800411C0.pos += D_800411C0.vel;
            if (D_800411C0.pos > 768) {
                D_800411C0.pos = 768;
                D_800411C0.vel = 0;
            } else if (D_800411C0.pos < -768) {
                D_800411C0.pos = -768;
                D_800411C0.vel = 0;
            }
        }
    } else {
        D_800411C0.timer--;
    }
}
