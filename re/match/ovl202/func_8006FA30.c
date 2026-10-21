#include "ac1/ac1.h"
/* ovl202 0x8006FA30 - turn an SVECTOR by q quarter turns about y */
void func_8006FA30(SVECTOR *in, SVECTOR *out, s32 q)
{
    switch (q) {
    case 0:
        out->vx = in->vx;
        out->vy = in->vy;
        out->vz = in->vz;
        break;
    case 1:
        out->vx = -in->vz;
        out->vy = in->vy;
        out->vz = in->vx;
        break;
    case 2:
        out->vx = -in->vx;
        out->vy = in->vy;
        out->vz = -in->vz;
        break;
    case 3:
        out->vx = in->vz;
        out->vy = in->vy;
        out->vz = -in->vx;
        break;
    }
}
