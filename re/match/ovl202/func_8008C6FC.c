#include "ac1/ac1.h"
#include "ac1/ovl202_t42.h"

void func_8008C6FC(SVECTOR *v) {
    SetRotMatrix(&SPAD->m);
    SetTransMatrix(&SPAD->m);
    SPAD->v = *v;
    SPAD->v.vx = (SPAD->v.vx >> 2) - SPAD->org.vx;
    SPAD->v.vy = (SPAD->v.vy >> 2) - SPAD->org.vy;
    SPAD->v.vz = (SPAD->v.vz >> 2) - SPAD->org.vz;
}
