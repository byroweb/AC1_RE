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
    u8 pad[0x14];
    u16 flags;          /* +0x14 */
    u16 zone;           /* +0x16 */
} AcDefZ;

extern Zone D_80198B08[];
extern void func_80078D78(AcEntity *e, void *d);

void func_8007909C(u32 *vis, AcEntity *e)
{
    AcDefZ *d;
    Zone *z;
    u16 fl;
    u16 sec;

    d = e->def;
    fl = d->flags;
    if (fl & 3) {
        z = &D_80198B08[d->zone];
        sec = e->section;
        if (vis[(s16)sec >> 5] & (1 << (sec & 0x1F))) {
            return;
        }
        if (e->pos.x < z->x + (z->lo[0] << 8) || z->x + (z->hi[0] << 8) < e->pos.x
            || e->pos.y < z->y + (z->lo[1] << 8) || z->y + (z->hi[1] << 8) < e->pos.y
            || e->pos.z < z->z + (z->lo[2] << 8) || z->z + (z->hi[2] << 8) < e->pos.z) {
            func_80078D78(e, e->def);
        }
        return;
    }
    if (fl & 0x80) {
        return;
    }
    sec = e->section;
    if (vis[(s16)sec >> 5] & (1 << (sec & 0x1F))) {
        return;
    }
    func_80078D78(e, d);
}
