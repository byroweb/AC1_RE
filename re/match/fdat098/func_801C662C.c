#include "ac1/ac1.h"

typedef struct {
    u8 st;              /* +0x00 */
    u8 idx;             /* +0x01 */
    u8 pad2[2];
    u16 x, y, z;        /* +0x04 */
    u8 pad0A[8];
    s16 ang;            /* +0x12 */
    u8 pad14[0x18];
} Slot;
typedef struct {
    u8 pad0[8];
    u16 x, y, z;        /* +0x08 */
    u8 padE[0x162];
} Ent;
typedef struct {
    u8 pad0[0x2C];
    Ent *hit;           /* +0x2C */
    u8 pad30[0x10];
} World;
typedef struct {
    u8 first;           /* +0x00 */
    u8 count;           /* +0x01 */
    u8 mode;            /* +0x02 */
} Def;
typedef struct {
    u8 pad0[0x28];
    u16 *ids;           /* +0x28 */
    u8 pad2c[0xC];
    Def *def;           /* +0x38 */
} Obj;
extern Slot D_8019FAB8[];
extern Ent D_801A26B8[];
extern World D_801D0B68[];
extern s16 func_8006E6B4();

void func_801C662C(Obj *o)
{
    Slot *s;
    s32 n;
    World *w;
    Def *d;

    d = o->def;
    s = &D_8019FAB8[d->first];
    n = d->count;
    w = &D_801D0B68[*o->ids];
    n--;
    if (n != -1) {
        do {
            if (s->st != 3) {
                if (s->st == 1) {
                    if (o->def->mode == 0) {
                        Ent *e = &D_801A26B8[s->idx];
                        e->x += w->hit->x;
                        e->y += w->hit->y;
                        e->z += w->hit->z;
                    }
                } else {
                    s->x += w->hit->x;
                    s->y += w->hit->y;
                    s->z += w->hit->z;
                    s->ang = func_8006E6B4(s->ang, (u8 *)s + 4);
                }
            }
            s++;
            n--;
        } while (n != -1);
    }
}
