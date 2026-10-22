#include "ac1/ac1.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    u8 pad06[0x1C];
    s8 lo[3];           /* +0x22 */
    s8 hi[3];           /* +0x25 */
    u8 pad28[0x0C];
} Zone;

typedef struct {
    u8 state;           /* +0x00 */
    u8 pad01[3];
    s16 x;              /* +0x04 */
    s16 y;              /* +0x06 */
    s16 z;              /* +0x08 */
    s16 h0a;            /* +0x0A */
    u8 pad0c[6];
    u16 section;        /* +0x12 */
    u16 flags;          /* +0x14 */
    u16 zone;           /* +0x16 */
} SpawnDef;

extern Zone D_80198B08[];
extern void func_80078CFC(SpawnDef *d);

void func_80078F00(u32 *vis, SpawnDef *d)
{
    Zone *z;
    u16 fl;
    u16 sec;

    if (d->h0a == -1) {
        return;
    }
    if (d->state != 0) {
        return;
    }
    fl = d->flags;
    if (fl & 0x40) {
        return;
    }
    if (fl & 1) {
        z = &D_80198B08[d->zone];
        sec = d->section;
        if (vis[(s16)sec >> 5] & (1 << (sec & 0x1F))) {
            goto spawn;
        }
        if (d->x < z->x + (z->lo[0] << 8) || z->x + (z->hi[0] << 8) < d->x
            || d->y < z->y + (z->lo[1] << 8) || z->y + (z->hi[1] << 8) < d->y
            || d->z < z->z + (z->lo[2] << 8) || z->z + (z->hi[2] << 8) < d->z) {
            return;
        }
        goto spawn;
    }
    sec = d->section;
    if (!(vis[(s16)sec >> 5] & (1 << (sec & 0x1F)))) {
        return;
    }
spawn:
    func_80078CFC(d);
}
