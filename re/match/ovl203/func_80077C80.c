#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

/* ovl203 copy of ovl202 func_80079370 (left/right pad scroll) on the current
 * player block.  The block pointer is fetched again after the timer store and
 * at the position update, as retail does. */
void func_80077C80(void) {
    VsPlayer *p;
    VsPlayer *q;
    u32 raw;
    u32 mask;
    u32 cur;
    s16 vel;
    s16 t;

    p = D_80039D18;
    mask = p->mask_l | p->mask_r;
    raw = p->pad_cur;
    cur = raw & mask;
    if (cur != (p->pad_prev & mask)) {
        p->scroll_timer = 0;
    }
    q = D_80039D18;
    t = q->scroll_timer;
    if (t == 0) {
        if ((q->pad_cur & (q->mask_l | q->mask_r)) == (q->mask_l | q->mask_r)) {
            q->scroll_vel = 0;
            q->scroll_pos = 0;
            q->scroll_timer = 10;
        } else {
            if (q->mask_l & q->pad_cur) {
                q->scroll_vel += 8;
                if (q->scroll_vel > 48) {
                    q->scroll_vel = 48;
                }
            } else if (q->pad_cur & q->mask_r) {
                q->scroll_vel -= 8;
                if (q->scroll_vel < -48) {
                    q->scroll_vel = -48;
                }
            } else {
                vel = q->scroll_vel;
                if (vel > 0) {
                    q->scroll_vel = vel - 16;
                    if (q->scroll_vel < 0) {
                        q->scroll_vel = 0;
                    }
                } else if (vel < 0) {
                    q->scroll_vel = vel + 16;
                    if (q->scroll_vel > 0) {
                        q->scroll_vel = 0;
                    }
                }
            }
            q = D_80039D18;
            q->scroll_pos += q->scroll_vel;
            if (q->scroll_pos > 768) {
                q->scroll_pos = 768;
                q->scroll_vel = 0;
            } else if (q->scroll_pos < -768) {
                q->scroll_pos = -768;
                q->scroll_vel = 0;
            }
        }
    } else {
        q->scroll_timer = t - 1;
    }
}
