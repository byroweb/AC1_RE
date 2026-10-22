#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x14];
    u16 flags;          /* +0x14 bit 15: slot may not be reused */
} EntDef;

extern void func_80078D78(AcEntity *e, EntDef *def);

AcEntity *ac_entity_alloc(void *def_)
{
    EntDef *def = def_;
    AcEntity *p;
    AcEntity *e;
    s32 i;
    s32 j;

    p = &g_ac_entities[1];
    i = 15;
    do {
        i--;
        if (p->def == 0) {
            return p;
        }
        p++;
    } while (i != 0);
    if (def->flags & 0x8000) {
        e = &g_ac_entities[1];
        j = 15;
        do {
            EntDef *d = e->def;
            if (d != 0) {
                if (!(d->flags & 0x8000)) {
                    func_80078D78(e, d);
                    return e;
                }
            }
            j--;
            e++;
        } while (j != 0);
    }
    return 0;
}
