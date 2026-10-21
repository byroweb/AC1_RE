#include "ac1/ac1.h"

typedef struct {
    u16 a;
    u8 b;
    u8 c;
    u8 d;
    s8 e;
    u8 f;
} Func801C68ACDef;
typedef struct {
    u8 pad0[0x18];
    u8 pos[8];          /* +0x18 */
    u8 rot[0x18];       /* +0x20 */
    Func801C68ACDef *def; /* +0x38 */
    u8 pad3C[4];
} Func801C68ACObj;      /* 0x40 */
extern Func801C68ACObj D_801D0B68[];
extern u32 D_801AC860[];
extern void func_8004D610();

void func_801C68AC(Func801C68ACObj *o)
{
    s32 i;
    Func801C68ACDef *d;

    i = o - D_801D0B68;
    if (D_801AC860[i >> 5] & (1 << (i & 0x1F))) {
        d = o->def;
        func_8004D610(o->pos, o->rot, d->a, d->b, d->c, d->d, d->e, d->f);
    }
}
