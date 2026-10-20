/* fdat000 (mission library, entry 0 @ 0x801C4B40; 21 twins): hit handler installed by
 * func_801C53D4.  When the world object hit last (0x801A26CE) is this one, and every
 * world object in the stats record's range (first +6, count +8) has lost its handler,
 * fire func_8008A0B0 with the record's arguments and drop the handler once it accepts. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x10];
    u16 world;          /* +0x10 */
} Func801C5314Hit;
typedef struct {
    u8 a;               /* +0x00 */
    u8 b;               /* +0x01 */
    u16 c;              /* +0x02 */
    u8 d;               /* +0x04 */
    u8 e;               /* +0x05 */
    u16 first;          /* +0x06 */
    u16 count;          /* +0x08 */
} Func801C5314Stats;
typedef struct {
    u8 pad0[0x2C];
    Func801C5314Hit *on_hit; /* +0x2C */
    u8 pad30[8];
    Func801C5314Stats *stats; /* +0x38 */
    u8 pad3c[4];
} Func801C5314World;
extern Func801C5314World D_801D0B68[];
extern s16 D_801A26CE;
extern s32 func_8008A0B0(s32 a, s32 b, s32 c, s32 d, s32 e);
void func_801C5314(Func801C5314World *w)
{
    Func801C5314Stats *s;
    s32 i, n;

    if (D_801A26CE == w->on_hit->world) {
        s = w->stats;
        i = s->first;
        if (i != 0) {
            n = s->count;
            for (n--; n != -1; n--) {
                if (D_801D0B68[i].on_hit != 0)
                    return;
                i++;
            }
        }
        if (func_8008A0B0(w->stats->a, w->stats->b, w->stats->c, w->stats->d, w->stats->e))
            w->on_hit = 0;
    }
}
