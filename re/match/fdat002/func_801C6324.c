/* fdat002 (17 twins): world-object trigger update (installed by func_801C63F8).  Ask
 * func_801C6064 about the stats +7 condition: on 1 start effect +8 at the record's
 * position (func_8004CC30, id 0x3000 + world index), on 2 sound +9 (func_8004CE44). */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x22];
    SVECTOR pos;        /* +0x22 */
} Func801C6324Hit;
typedef struct {
    u8 pad0[7];
    u8 cond;            /* +0x07 */
    u8 effect;          /* +0x08 */
    u8 sound;           /* +0x09 */
} Func801C6324Stats;
typedef struct {
    u8 pad0[0x2C];
    Func801C6324Hit *on_hit; /* +0x2C */
    u8 pad30[8];
    Func801C6324Stats *stats; /* +0x38 */
    u8 pad3c[4];
} Func801C6324World;
extern Func801C6324World D_801D0B68[];
extern s32 func_801C6064(Func801C6324World *w, s32 cond);
extern void func_8004CC30(s32 effect, s32 id, SVECTOR *pos, s32 a, s32 b, s32 c, s32 d);
extern void func_8004CE44(s32 sound, SVECTOR *pos, s32 vol, s32 d);
void func_801C6324(Func801C6324World *w)
{
    switch (func_801C6064(w, w->stats->cond)) {
    case 1:
        if (w->stats->effect != 0)
            func_8004CC30(w->stats->effect, (w - D_801D0B68) + 0x3000, &w->on_hit->pos, 100, 8000,
                          10000, 0);
        break;
    case 2:
        if (w->stats->sound != 0)
            func_8004CE44(w->stats->sound, &w->on_hit->pos, 120, 0);
        break;
    }
}
