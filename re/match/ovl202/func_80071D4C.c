#include "ac1/ac1.h"
/* ovl202 0x80071D4C - nearest probe hit (smallest dist) whose polygon classes miss mask.
 * The hit list at 0x801E8B74 is 28-byte records ending at poly == 0 (the CollHit
 * header in collide.h covers only the first 8 bytes of record 0). */
typedef struct HitRec {
    CollPoly *poly;             /* +0x00 0 ends the list */
    u8 unk04[0xA];
    s16 dist;                   /* +0x0E */
    u8 unk10[0xC];
} HitRec;
extern HitRec D_801E8B74[];

HitRec *func_80071D4C(s32 mask)
{
    s32 best;
    HitRec *r;
    HitRec *res;

    best = 0x7FFFFFFF;
    r = D_801E8B74;
    res = 0;
    while (r->poly != 0) {
        if (!(r->poly->v01.h.hi & 7 & mask)) {
            if (r->dist < best) {
                best = r->dist;
                res = r;
            }
        }
        r++;
    }
    return res;
}
