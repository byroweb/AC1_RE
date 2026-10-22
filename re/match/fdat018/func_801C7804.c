#include "ac1/ac1.h"

typedef struct {
    u8 state;           /* +0x00 */
    u8 pad1[0x2B];
} Slot2C;               /* 0x2C */
typedef struct {
    Slot2C slots[256];              /* 0x8019FAB8 */
    AcEntity ents[16];              /* 0x801A26B8 */
    u8 pad[0x200C];
    s32 frame;                      /* 0x801A5DC4 */
} World;
typedef struct {
    u8 first;
    u8 count;
} Range;
typedef struct {
    u8 pad0[0x38];
    Range *range;
} Owner;
extern World D_8019FAB8;

void func_801C7804(Owner *o)
{
    s32 n;
    Slot2C *s;
    AcEntity *e;
    s32 i;

    if ((D_8019FAB8.frame & 7) == 0) {
        s = &D_8019FAB8.slots[o->range->first];
        n = o->range->count;
        while (--n != -1) {
            if (s->state == 1) {
                goto set;
            }
            s++;
        }
        e = D_8019FAB8.ents;
        i = 16;
        do {
            if (e->def != 0 && (e->faction & 7) == 2) {
                e->ignore_mask &= 0xFFFE;
            }
            e++;
            i--;
        } while (i != 0);
        return;
set:
        e = D_8019FAB8.ents;
        i = 16;
        do {
            if (e->def != 0 && (e->faction & 7) == 2) {
                e->ignore_mask |= 1;
            }
            e++;
            i--;
        } while (i != 0);
    }
}
