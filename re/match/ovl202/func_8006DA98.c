#include "ac1/ac1.h"
/* ovl202 0x8006DA98 - nearest unmasked probe hit (|d| Manhattan, minus 64 per surface
 * class step), its hit point turned back by the world struct's quarter turns */
typedef struct HitRec {
    CollPoly *poly;             /* +0x00 */
    WorldStruct *ws;            /* +0x04 */
    CollMesh *mesh;             /* +0x08 */
    SVECTOR d;                  /* +0x0C */
    SVECTOR pt;                 /* +0x14 */
} HitRec;
extern HitRec D_801E8B74[];
void func_8006FA30(SVECTOR *in, SVECTOR *out, s32 q);

HitRec *func_8006DA98(s32 mask)
{
    s32 best;
    s32 d;
    HitRec *r;
    HitRec *res;
    CollPoly *p;

    best = 0x7FFFFFFF;
    res = 0;
    for (r = D_801E8B74; r->poly != 0; r++) {
        p = r->poly;
        if (!(p->v01.h.hi & 7 & mask)) {
            d = abs(r->d.vx) + abs(r->d.vy) + abs(r->d.vz) - ((p->v23 >> 16 & 7) << 6);
            if (d < best) {
                best = d;
                res = r;
            }
        }
    }
    if (res != 0) {
        func_8006FA30(&res->d, &res->pt, res->ws->unk16);
    }
    return res;
}
