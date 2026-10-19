/* ovl202 func_80079370 - left/right pad scroll: while a held direction (maskL /
 * maskR at +0x29E/+0x29A of the pad block D_80040FA8) is pressed, vel ramps by 8
 * up to +-48, otherwise decays by 16 toward 0; pos += vel clamped to +-768; a new
 * button state restarts the timer, both buttons together reset pos/vel and hold
 * for 10 frames.
 *
 * Address forms (see the match-function skill, "base register" rows):
 *  - pad: retail reads maskL/maskR/cur as 670/666/0 off one lui/addiu base.  That
 *    needs cur read first, the masks read in the entry block, cur read again
 *    (btn) at the top of the timer==0 block, and a store between them that the
 *    first CSE pass cannot see through: the block-local pointer store
 *    `sc->timer = 0`.  The rerun of CSE then turns btn into a copy of raw
 *    (the `move v0,a1` in the bne delay slot) but no longer refolds the base.
 *  - prev, vel and timer are direct lui/%lo accesses everywhere: scalar
 *    externs at the field addresses (a struct field read would join the base).
 *  - pos += vel and the clamp use the struct: that block reads both through a
 *    base materialised at the join, exactly as retail.
 * The empty 16-byte frame comes from the s16 values live across branches (vel, t). */
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
extern u32 D_80040FAC;   /* = D_80040FA8.prev, accessed as a scalar */
extern Func80079370Scroll D_800411C0;
extern s16 D_800411C2;   /* = D_800411C0.vel, accessed as a scalar */
extern s16 D_800411C4;   /* = D_800411C0.timer, accessed as a scalar */
void func_80079370(void)
{
    u32 raw;
    u32 mask;
    u32 cur;
    u32 btn;
    s16 vel;
    s16 t;
    raw = D_80040FA8.cur;
    mask = D_80040FA8.maskL | D_80040FA8.maskR;
    cur = raw & mask;
    if (cur != (D_80040FAC & mask)) {
        Func80079370Scroll *sc = &D_800411C0;
        sc->timer = 0;
    }
    t = D_800411C4;
    if (t == 0) {
        btn = D_80040FA8.cur;
        if (cur == mask) {
            D_800411C2 = 0;
            D_800411C0.pos = 0;
            D_800411C4 = 10;
        } else {
            if (D_80040FA8.maskL & btn) {
                D_800411C2 += 8;
                if (D_800411C2 > 48) {
                    D_800411C2 = 48;
                }
            } else if (D_80040FA8.cur & D_80040FA8.maskR) {
                D_800411C2 -= 8;
                if (D_800411C2 < -48) {
                    D_800411C2 = -48;
                }
            } else {
                vel = D_800411C2;
                if (vel > 0) {
                    D_800411C2 = vel - 16;
                    if (D_800411C2 < 0) {
                        D_800411C2 = 0;
                    }
                } else if (vel < 0) {
                    D_800411C2 = vel + 16;
                    if (D_800411C2 > 0) {
                        D_800411C2 = 0;
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
        D_800411C4 = t - 1;
    }
}
