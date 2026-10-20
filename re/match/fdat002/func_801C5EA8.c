/* fdat002 (17 twins): set the hit state of a chain of world objects.  Starting at w,
 * write v to the handler record's +0x10 byte (and clear the stats +6 byte for v == 4),
 * then step to the next world struct named by w's id list (+0x28, entries from [1]),
 * count from w +0x26. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x10];
    u8 state;           /* +0x10 */
} Func801C5EA8Hit;
typedef struct {
    u8 pad0[6];
    u8 b6;              /* +0x06 */
} Func801C5EA8Stats;
typedef struct {
    u8 pad0[0x26];
    s16 count;          /* +0x26 */
    u16 *ids;           /* +0x28 */
    Func801C5EA8Hit *on_hit; /* +0x2C */
    u8 pad30[8];
    Func801C5EA8Stats *stats; /* +0x38 */
    u8 pad3c[4];
} Func801C5EA8World;
extern Func801C5EA8World D_801D0B68[];
void func_801C5EA8(Func801C5EA8World *w, s32 v)
{
    s32 n = w->count;
    u16 *id = w->ids + 1;

    do {
        w->on_hit->state = v;
        if (v == 4)
            w->stats->b6 = 0;
        w = &D_801D0B68[*id++];
    } while (--n != 0);
}
