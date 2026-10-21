#include "ac1/ac1.h"

extern AcEntity D_8019B550[2];

/* The actor whose definition record is def, or 0. */
AcEntity *func_8007785C(void *def) {
    AcEntity *e = D_8019B550;
    s32 i;

    i = 2;
    do {
        if (e->def == def) {
            return e;
        }
        e++;
    } while (--i);
    return 0;
}
