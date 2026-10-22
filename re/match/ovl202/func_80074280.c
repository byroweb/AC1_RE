#include "ac1/ac1.h"

typedef struct {
    u8 b[0x34];
} Obj34;

extern Obj34 D_80198B08[];

void *func_80074280(void *p)
{
    if ((AcEntity *)p >= &g_ac_entities[0] && (AcEntity *)p <= &g_ac_entities[15]) {
        return &((AcEntity *)p)->pos;
    }
    if ((Obj34 *)p >= &D_80198B08[0] && (Obj34 *)p <= &D_80198B08[511]) {
        return p;
    }
    return 0;
}
