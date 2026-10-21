#include "ac1/ac1.h"
/* ovl202 0x80071188 - world y of two corners of the hit polygon: corner slots picked
 * by info bits 6-7 and 12-13, vertex y >> 2 plus the world struct origin y */
typedef struct PolyV {
    u32 info;
    u8 unk04[8];
    u16 corner[4];              /* +0x0C */
} PolyV;
typedef struct HitRec {
    PolyV *poly;                /* +0x00 */
    WorldStruct *ws;            /* +0x04 */
    CollMesh *mesh;             /* +0x08 */
    u8 unk0C[2];
    s16 dist;                   /* +0x0E */
    u8 unk10[0xC];
} HitRec;


void func_80071188(HitRec *h, s16 *a, s16 *b)
{
    PolyV *p;
    s32 verts;
    s32 c;

    p = h->poly;
    c = p->corner[(p->info >> 6) & 3];
    verts = (s32)h->mesh->verts;
    *a = h->ws->org[1] + (((SVECTOR *)((c & 0x7FF8) + verts))->vy >> 2);
    p = h->poly;
    c = p->corner[(p->info >> 12) & 3];
    *b = h->ws->org[1] + (((SVECTOR *)((c & 0x7FF8) + verts))->vy >> 2);
}
