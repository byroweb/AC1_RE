#include "ac1/ac1.h"
#include "ac1/ovl202_t42.h"
#include "ac1/gte.h"

void func_8008C7BC(SVECTOR *in, ScrPt *out, s32 n) {
    do {
        gte_ldv0(in);
        gte_rtps();
        in++;
        gte_stsxy(&out->x);
        gte_stszotz(&out->z);
        if (out->z == 0) {
            out->clip = 255;
        } else {
            if (out->x < 0) {
                out->clip = 1;
            } else if (out->x >= 320) {
                out->clip = 2;
            } else {
                out->clip = 0;
            }
            if (out->y < 0) {
                out->clip |= 4;
            } else if (out->y >= 240) {
                out->clip |= 8;
            }
        }
        out++;
    } while (--n);
}
