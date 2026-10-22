#include "ac1/ac1.h"

extern AcEntity *D_801A5DBC[];
extern void *D_801A5DC0[];
extern s16 func_8006E6B4(s32 section, AcPos *pos);

void ac_entity_update(AcEntity *e, void *ctx)
{
    AcPos save;

    save = e->pos;
    D_801A5DBC[0] = e;
    D_801A5DC0[0] = ctx;
    if (e->fn_update != 0) {
        e->fn_update(e);
        e->section = func_8006E6B4(e->section, &e->pos);
    }
    if (e->fn_think != 0) {
        e->fn_think(e);
    }
    e->prev_pos = save;
}
