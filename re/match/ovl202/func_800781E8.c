#include "ac1/ac1.h"

typedef struct {
    void *unk0;
    s16 *info;
} LosHit;

extern LosHit *func_8007816C(AcEntity *e, AcEntity *t, SVECTOR *out);

s32 func_800781E8(AcEntity *e)
{
    AcEntity *t;
    LosHit *h;
    SVECTOR v;

    t = e->target;
    if (t == 0) {
        return 0;
    }
    if (t->def == 0) {
        return 0;
    }
    h = func_8007816C(e, t, &v);
    if (h == 0) {
        return 1;
    }
    return h->info[3] == 0x30;
}
