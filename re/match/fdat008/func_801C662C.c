#include "ac1/ac1.h"

typedef struct {
    u8 kind;            /* +0x00 */
    u8 ent;             /* +0x01 */
    u8 pad2[0x2A];
} Func801C662CSlot;     /* 0x2C */
typedef struct {
    Func801C662CSlot slots[256];    /* 0x8019FAB8 */
    AcEntity ents[16];              /* 0x801A26B8 */
} Func801C662CWorld;
typedef struct {
    u16 count;          /* +0x00 */
    u8 slot;            /* +0x02 */
    u8 a;               /* +0x03 */
    u8 b;               /* +0x04 */
    u8 pad5;
    u16 c;              /* +0x06 */
    u8 d;               /* +0x08 */
    u8 e;               /* +0x09 */
} Func801C662CDef;
typedef struct {
    u8 pad0[0x2C];
    s32 timer;          /* +0x2C */
    u8 pad30[8];
    Func801C662CDef *def; /* +0x38 */
} Func801C662CObj;
extern Func801C662CWorld D_8019FAB8;
extern s32 func_8006D45C(Func801C662CObj *);
extern void func_8008A0B0(s32, s32, s32, s32, s32);

void func_801C662C(Func801C662CObj *o)
{
    Func801C662CDef *d;
    Func801C662CSlot *s;
    AcEntity *e;

    d = o->def;
    s = &D_8019FAB8.slots[d->slot];
    if (s->kind == 3) {
    fire:
        o->timer = 0;
        if (func_8006D45C(o) != 0) {
            func_8008A0B0(d->a, d->b, d->c, d->d, d->e);
        }
    } else if (s->kind == 1) {
        e = &D_8019FAB8.ents[s->ent];
        if ((d->count == 1 && e->ai_mode != 0xFF) || e->ap < d->count - 1) {
            goto fire;
        }
    }
}
