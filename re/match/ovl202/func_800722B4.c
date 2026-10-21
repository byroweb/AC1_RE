#include "ac1/ac1.h"
/* ovl202 0x800722B4 - probe one world struct with an SVECTOR segment: if it overlaps
 * the struct's box, walk the mesh buckets it spans and append hit records (mode as
 * func_8006F08C); v, if given, is turned into mesh space too */
typedef struct HitRec {
    CollPoly *poly;             /* +0x00 */
    WorldStruct *ws;            /* +0x04 */
    CollMesh *mesh;             /* +0x08 */
    SVECTOR d;                  /* +0x0C */
    SVECTOR pt;                 /* +0x14 */
} HitRec;
typedef struct BoxScr {         /* 0x1F800000 during a box probe */
    u32 bits[18];               /* +0x00 */
    HitRec *out;                /* +0x48 next hit record */
    s32 mode;                   /* +0x4C */
    SVECTOR res;                /* +0x50 */
    SVECTOR v;                  /* +0x58 */
    SVECTOR *vout;              /* +0x60 &v or 0 */
    SVECTOR seg[2];             /* +0x64 */
    SVECTOR box[2];             /* +0x74 */
    CollMesh *meshes;           /* +0x84 */
    s32 idx;                    /* +0x88 */
    s32 group;                  /* +0x8C */
    WorldStruct *ws;            /* +0x90 */
    CollMesh *mesh;             /* +0x94 */
    CollHdr *hdr;               /* +0x98 */
    CollPoly poly;              /* +0x9C */
    CollPoly *cur;              /* +0xB0 */
    CollPoly *end;              /* +0xB4 */
    CollPoly *group_end;        /* +0xB8 */
    u32 mask;                   /* +0xBC */
} BoxScr;
extern HitRec D_801E8B74[];
extern CollMesh D_80091228[];
s32 func_8006EFFC(s32 a0, s32 a1, s32 b0, s32 b1);
void func_8006F83C(BoxScr *s, SVECTOR *seg, SVECTOR *v);
u32 func_8006F7A0(u32 *tab, s32 lo, s32 hi, s32 n);
void func_8006F08C(BoxScr *s);

s32 func_800722B4(WorldStruct *ws, SVECTOR *seg, SVECTOR *v, s32 mode)
{
    BoxScr *s;

    s = (BoxScr *)0x1F800000;
    if (!func_8006EFFC(seg[0].vx, seg[1].vx, ws->bbox_min[0], ws->bbox_max[0])
        || !func_8006EFFC(seg[0].vy, seg[1].vy, ws->bbox_min[1], ws->bbox_max[1])
        || !func_8006EFFC(seg[0].vz, seg[1].vz, ws->bbox_min[2], ws->bbox_max[2])) {
        return 0;
    }
    s->out = D_801E8B74;
    s->mode = mode;
    s->meshes = D_80091228;
    s->ws = ws;
    if (v == 0) {
        s->vout = 0;
    } else {
        s->vout = &s->v;
    }
    func_8006F83C(s, seg, v);
    s->mesh = &s->meshes[s->ws->mesh];
    s->hdr = s->mesh->hdr;
    s->mask = func_8006F7A0((u32 *)s->hdr + 1, s->seg[0].vx - s->box[0].vx, s->seg[1].vx - s->box[0].vx, s->box[1].vx - s->box[0].vx);
    s->mask &= func_8006F7A0((u32 *)s->hdr + 9, s->seg[0].vy - s->box[0].vy, s->seg[1].vy - s->box[0].vy, s->box[1].vy - s->box[0].vy);
    s->mask &= func_8006F7A0((u32 *)s->hdr + 17, s->seg[0].vz - s->box[0].vz, s->seg[1].vz - s->box[0].vz, s->box[1].vz - s->box[0].vz);
    if (s->mask != 0) {
        s->cur = (CollPoly *)(s->hdr + 1);
        s->end = s->cur + s->hdr->n;
        s->group = (s->hdr->n >> 5) + 1;
        func_8006F08C(s);
    }
    s->out->poly = 0;
    return s->out != D_801E8B74;
}
