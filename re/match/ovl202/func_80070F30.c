#include "ac1/ac1.h"
/* ovl202 0x80070F30 - probe from -> to: against world struct ws (the nearest hit not
 * masked out, its point to *hit), or with ws == -1 the first unmasked hit of the list */
typedef struct HitRec {
    CollPoly *poly;             /* +0x00 */
    WorldStruct *ws;            /* +0x04 */
    CollMesh *mesh;             /* +0x08 */
    u8 unk0C[2];
    s16 dist;                   /* +0x0E */
    u8 unk10[4];
    SVECTOR pt;                 /* +0x14 */
} HitRec;
extern HitRec D_801E8B74[];
void func_8006F4D8(SVECTOR *from, SVECTOR *to, s32 ws);
HitRec *func_8006DA98(s32 mask);

HitRec *func_80070F30(SVECTOR *from, SVECTOR *to, SVECTOR *hit, s32 ws, s32 mask)
{
    HitRec *r;
    HitRec *q;

    func_8006F4D8(from, to, ws);
    if (ws != -1) {
        r = func_8006DA98(mask);
        if (r == 0) {
            return 0;
        }
        if (hit != 0) {
            *hit = r->pt;
        }
        return r;
    }
    for (q = D_801E8B74; q->poly != 0; q++) {
        if (!(q->poly->v01.h.hi & 7 & mask)) {
            return q;
        }
    }
    return 0;
}
