/* fdat002 (16 twins): world-object trigger event hook (installed by func_801C63F8),
 * func(w, event, arg, ...).  0x8100 (touched by an object of kind 2 while the state is
 * below 4): play stats +6 through func_800529F0 if set, else trigger with state 1;
 * 0x8200 triggers with state 1, 0x8201 with state 4.  Triggering (while +0x11 < +0x12)
 * clears flag 0x100 on the first linked world object and sets the chain's state
 * (func_801C5EA8).  Returns 1 when handled. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x0C];
    u32 flags;          /* +0x0C */
} Func801C5EF8Thing;
typedef struct {
    u8 pad0[0x10];
    u8 state;           /* +0x10 */
    u8 count;           /* +0x11 */
    u8 limit;           /* +0x12 */
} Func801C5EF8Hit;
typedef struct {
    u8 pad0[6];
    u8 sound;           /* +0x06 */
} Func801C5EF8Stats;
typedef struct {
    u8 pad0[0x28];
    u16 *ids;           /* +0x28 */
    Func801C5EF8Hit *on_hit; /* +0x2C */
    u32 flags;          /* +0x30 */
    u8 pad34[4];
    Func801C5EF8Stats *stats; /* +0x38 */
    u8 pad3c[4];
} Func801C5EF8World;
extern Func801C5EF8World D_801D0B68[];
extern void func_800529F0(s32 a, s32 b, s32 sound);
extern void func_801C5EA8(Func801C5EF8World *w, s32 state);
s32 func_801C5EF8(Func801C5EF8World *w, s32 event, Func801C5EF8Thing **arg, ...)
{
    Func801C5EF8World *link;
    s32 state;

    switch (event) {
    case 0x8100:
        if (w->on_hit->state >= 4)
            return 0;
        if (((*arg)->flags & 7) != 2)
            return 0;
        if (w->stats->sound != 0) {
            func_800529F0(0, 0, w->stats->sound);
            return 1;
        }
        state = 1;
        goto trigger;
    case 0x8201:
        state = 4;
        goto trigger;
    case 0x8200:
        state = 1;
    trigger:
        if (w->on_hit->count < w->on_hit->limit) {
            link = &D_801D0B68[*w->ids];
            link->flags &= ~0x100;
            func_801C5EA8(w, state);
        }
        return 1;
    }
    return 0;
}
