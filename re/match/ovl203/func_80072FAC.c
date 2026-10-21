#include "ac1/ac1.h"

typedef struct {
    SVECTOR pos;                /* +0x00 */
    u8 unk08[0x34 - 0x08];
} Ovl203Obj;

extern AcEntity D_8019B550[2];
extern Ovl203Obj D_80194A70[512];

/* Position of an actor or pool object, 0 for anything else. */
void *func_80072FAC(void *obj) {
    if ((AcEntity *)obj >= D_8019B550 && (AcEntity *)obj <= &D_8019B550[1]) {
        return &((AcEntity *)obj)->pos;
    }
    if ((Ovl203Obj *)obj >= D_80194A70 && (Ovl203Obj *)obj <= &D_80194A70[511]) {
        return obj;
    }
    return 0;
}
