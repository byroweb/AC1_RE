#include "ac1/ac1.h"

typedef struct {
    u8 b[0x34];
} Obj34;

extern Obj34 D_80198B08[];

s32 func_800742DC(void *p, SVECTOR *out)
{
    AcEntity *e;

    if ((AcEntity *)p >= &g_ac_entities[0] && (AcEntity *)p <= &g_ac_entities[15]) {
        e = p;
        if (e->def == 0) {
            return 0;
        }
        *out = *(SVECTOR *)&e->pos;
        out->vy -= *(u16 *)((u8 *)e + 0x6C) >> 1;
        return 1;
    }
    if ((Obj34 *)p >= &D_80198B08[0] && (Obj34 *)p <= &D_80198B08[511]) {
        *out = *(SVECTOR *)p;
        return 1;
    }
    return 0;
}
