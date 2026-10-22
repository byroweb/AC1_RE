#include "ac1/ac1.h"

AcEntity *ac_entity_find_def(void *def)
{
    AcEntity *e;
    s32 i;

    e = g_ac_entities;
    i = 16;
    do {
        if (e->def == def) {
            return e;
        }
        e++;
    } while (--i);
    return 0;
}
