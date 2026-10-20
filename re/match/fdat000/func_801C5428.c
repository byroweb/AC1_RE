/* fdat000 (mission library, entry 0 @ 0x801C4B40; 20 twins): "all destroyed" trigger.
 * Walk the stats record's range of spawn records (0x8019FAB8, 44 bytes each): an AC
 * (kind 1) still counts while its AP is at least stats +0 - 1 (or, for +0 == 1, while
 * its AI mode is 255); kinds other than 1 and 3 always block.  When nothing is left,
 * drop the handler and, if func_8006D45C allows, fire func_8008A0B0. */
#include "ac1/ac1.h"
typedef struct {
    u8 kind;            /* +0x00 */
    u8 slot;            /* +0x01 */
    u8 pad2[8];
    s16 id;             /* +0x0A -1 = empty */
    u8 padc[0x2C - 0x0C];
} Func801C5428Spawn;
typedef struct {
    u16 left;           /* +0x00 */
    u8 first;           /* +0x02 */
    u8 count;           /* +0x03 */
    u8 a;               /* +0x04 */
    u8 b;               /* +0x05 */
    u16 c;              /* +0x06 */
    u8 d;               /* +0x08 */
    u8 e;               /* +0x09 */
} Func801C5428Stats;
typedef struct {
    u8 pad0[0x2C];
    void *on_hit;       /* +0x2C */
    u8 pad30[8];
    Func801C5428Stats *stats; /* +0x38 */
} Func801C5428World;
/* The spawn table sits right before g_ac_entities (0x801A26B8): retail addresses both
 * from one base, so they are one block here. */
typedef struct {
    Func801C5428Spawn spawn[256]; /* +0x0000 */
    AcEntity ents[16];  /* +0x2C00 = g_ac_entities */
} Func801C5428Block;
extern Func801C5428Block D_8019FAB8;
extern s32 func_8006D45C(Func801C5428World *w);
extern s32 func_8008A0B0(s32 a, s32 b, s32 c, s32 d, s32 e);
void func_801C5428(Func801C5428World *w)
{
    Func801C5428Stats *s = w->stats;
    Func801C5428Spawn *r;
    AcEntity *e;
    s32 n;

    n = s->count;
    r = &D_8019FAB8.spawn[s->first];
    do {
        if (r->id != -1) {
            if (r->kind == 1) {
                e = &D_8019FAB8.ents[r->slot];
                if (s->left == 1 && e->ai_mode == 255)
                    return;
                if (e->ap >= s->left - 1)
                    return;
            } else if (r->kind != 3) {
                return;
            }
        }
        r++;
    } while (--n != 0);
    w->on_hit = 0;
    if (func_8006D45C(w))
        func_8008A0B0(s->a, s->b, s->c, s->d, s->e);
}
