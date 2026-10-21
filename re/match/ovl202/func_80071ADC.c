#include "ac1/ac1.h"
/* ovl202 0x80071ADC - segment probe: for every candidate world struct whose box the
 * segment overlaps, walk the mesh buckets the segment spans and append hit records
 * to D_801E8B74 (func_80071768); 1 if anything was hit.  func_800705D8's sibling. */
typedef struct HitRec {
    CollPoly *poly;             /* +0x00 */
    WorldStruct *ws;            /* +0x04 */
    CollMesh *mesh;             /* +0x08 */
    SVECTOR d;                  /* +0x0C */
    SVECTOR pt;                 /* +0x14 */
} HitRec;
typedef struct SegScr {         /* 0x1F800000 during a segment probe */
    u32 bits[18];               /* +0x00 candidate world structs; +0x40 / +0x44 row width / gap */
    HitRec *out;                /* +0x48 next hit record */
    s32 mode;                   /* +0x4C */
    u8 unk50[0xC];
    s32 min[3];                 /* +0x5C segment start, mesh space */
    s32 max[3];                 /* +0x68 segment end */
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
} SegScr;
extern HitRec D_801E8B74[];
extern CollMesh D_80091228[];
extern s32 D_801E8B70;
void func_8006DEF4(SVECTOR *seg, SegScr *s);
s32 func_8006FB6C(void *bits, ...);
s32 func_8006EFFC(s32 a0, s32 a1, s32 b0, s32 b1);
void func_80071904(SegScr *s, SVECTOR *seg);
u32 func_8006F7A0(u32 *tab, s32 lo, s32 hi, s32 n);
void func_80071768(SegScr *s);

s32 func_80071ADC(SVECTOR *seg, s32 mode)
{
    SegScr *s;

    s = (SegScr *)0x1F800000;
    func_8006DEF4(seg, s);
    s->out = D_801E8B74;
    s->mode = mode;
    s->meshes = D_80091228;
    for (s->idx = func_8006FB6C(s, D_801E8B70); s->idx != -1; s->idx = func_8006FB6C(0)) {
        s->ws = &g_world_structs[s->idx];
        if (func_8006EFFC(seg[0].vx, seg[1].vx, s->ws->bbox_min[0], s->ws->bbox_max[0])
            && func_8006EFFC(seg[0].vy, seg[1].vy, s->ws->bbox_min[1], s->ws->bbox_max[1])
            && func_8006EFFC(seg[0].vz, seg[1].vz, s->ws->bbox_min[2], s->ws->bbox_max[2])) {
            func_80071904(s, seg);
            s->mesh = &s->meshes[s->ws->mesh];
            s->hdr = s->mesh->hdr;
            s->mask = func_8006F7A0((u32 *)s->hdr + 1, s->min[0] - s->box[0].vx, s->max[0] - s->box[0].vx, s->box[1].vx - s->box[0].vx);
            s->mask &= func_8006F7A0((u32 *)s->hdr + 9, s->min[1] - s->box[0].vy, s->max[1] - s->box[0].vy, s->box[1].vy - s->box[0].vy);
            s->mask &= func_8006F7A0((u32 *)s->hdr + 17, s->min[2] - s->box[0].vz, s->max[2] - s->box[0].vz, s->box[1].vz - s->box[0].vz);
            if (s->mask != 0) {
                s->cur = (CollPoly *)(s->hdr + 1);
                s->end = s->cur + s->hdr->n;
                s->group = (s->hdr->n >> 5) + 1;
                func_80071768(s);
            }
        }
    }
    s->out->poly = 0;
    return s->out != D_801E8B74;
}
