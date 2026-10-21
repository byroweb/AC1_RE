#include "ac1/ac1.h"

typedef struct {
    AcPos pos;                  /* +0x00 */
    u8 unk08[0x34 - 0x08];
} Ovl203Obj;

extern AcEntity D_8019B550[2];
extern Ovl203Obj D_80194A70[512];

/* Aim point of an actor (its position raised by half its +0x6C height) or pool object. */
s32 func_80073008(void *obj, AcPos *out) {
    if ((AcEntity *)obj >= D_8019B550 && (AcEntity *)obj <= &D_8019B550[1]) {
        if (((AcEntity *)obj)->def == 0) {
            return 0;
        }
        *out = ((AcEntity *)obj)->pos;
        out->y -= *(u16 *)((u8 *)obj + 0x6C) >> 1;
        return 1;
    }
    if ((Ovl203Obj *)obj >= D_80194A70 && (Ovl203Obj *)obj <= &D_80194A70[511]) {
        *out = ((Ovl203Obj *)obj)->pos;
        return 1;
    }
    return 0;
}
