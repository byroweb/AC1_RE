#include "ac1/ac1.h"

typedef struct {
    u8 state;           /* +0x00 */
    u8 pad01;
    u16 ap;             /* +0x02 */
    s16 x;              /* +0x04 */
    s16 y;              /* +0x06 */
    s16 z;              /* +0x08 */
    u8 pad0a[2];
    SVECTOR rot;        /* +0x0C */
    u8 pad14[0];
} AcDefSave;

typedef struct {
    u8 pad[0x14];
    u16 flags;          /* +0x14 */
} AcDefFlags;

extern void func_800558A8(s32 h);

void func_80078D78(AcEntity *e, AcDefSave *d)
{
    d->state = 0;
    if (!(((AcDefFlags *)d)->flags & 0x10)) {
        d->ap = e->ap;
        d->x = e->pos.x;
        d->y = e->pos.y;
        d->z = e->pos.z;
        d->rot = *(SVECTOR *)e->unk10;
    }
    if (e->unk60 != 0) {
        func_800558A8(e->unk60);
    }
    e->def = 0;
}
