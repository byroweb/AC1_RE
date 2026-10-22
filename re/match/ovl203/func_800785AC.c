#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

/* ovl203 copy of ovl202 func_80079AF8: decay two velocities toward zero by the
 * current player's rate (read afresh at every use). */
typedef struct {
    u8 pad[0xCC];
    s16 vx;             /* +0xCC */
    s16 padCE;
    s16 vz;             /* +0xD0 */
} Func800785ACObj;

#define DECAY(p)                                        \
    {                                                   \
        s16 v;                                          \
                                                        \
        v = (p);                                        \
        if (v > 0) {                                    \
            if (v > D_80039D18->decel) {                \
                (p) = v - D_80039D18->decel;            \
            } else {                                    \
                (p) = 0;                                \
            }                                           \
        } else {                                        \
            if (v < -D_80039D18->decel) {               \
                (p) = v + D_80039D18->decel;            \
            } else {                                    \
                (p) = 0;                                \
            }                                           \
        }                                               \
    }

void func_800785AC(Func800785ACObj *o) {
    s32 unused[2];

    DECAY(o->vx);
    DECAY(o->vz);
}
