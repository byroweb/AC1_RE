#include "ac1/ac1.h"
/* ovl202 0x8006F83C - func_80071904 for the SVECTOR probe record: segment ends and box
 * into a world struct's mesh space, both ordered (func_8006FB14); v too if given */
typedef struct Probe2 {
    u8 unk00[0x60];
    SVECTOR *vout;              /* +0x60 */
    SVECTOR seg[2];             /* +0x64 */
    SVECTOR box[2];             /* +0x74 */
    u8 unk84[0xC];
    WorldStruct *ws;            /* +0x90 */
    u8 unk94[0x50];
    SVECTOR ts;                 /* +0xE4 */
} Probe2;
void func_8006FA30(SVECTOR *in, SVECTOR *out, s32 q);
void func_8006FB14(SVECTOR *b);

void func_8006F83C(Probe2 *s, SVECTOR *pts, SVECTOR *v)
{
    s32 q;

    q = -s->ws->unk16 & 3;
    s->ts.vx = (pts[0].vx - s->ws->org[0]) << 2;
    s->ts.vy = (pts[0].vy - s->ws->org[1]) << 2;
    s->ts.vz = (pts[0].vz - s->ws->org[2]) << 2;
    func_8006FA30(&s->ts, &s->seg[0], q);
    s->ts.vx = (pts[1].vx - s->ws->org[0]) << 2;
    s->ts.vy = (pts[1].vy - s->ws->org[1]) << 2;
    s->ts.vz = (pts[1].vz - s->ws->org[2]) << 2;
    func_8006FA30(&s->ts, &s->seg[1], q);
    func_8006FB14(s->seg);
    s->ts.vx = (s->ws->bbox_min[0] - s->ws->org[0]) << 2;
    s->ts.vy = (s->ws->bbox_min[1] - s->ws->org[1]) << 2;
    s->ts.vz = (s->ws->bbox_min[2] - s->ws->org[2]) << 2;
    func_8006FA30(&s->ts, &s->box[0], q);
    s->ts.vx = (s->ws->bbox_max[0] - s->ws->org[0]) << 2;
    s->ts.vy = (s->ws->bbox_max[1] - s->ws->org[1]) << 2;
    s->ts.vz = (s->ws->bbox_max[2] - s->ws->org[2]) << 2;
    func_8006FA30(&s->ts, &s->box[1], q);
    func_8006FB14(s->box);
    if (v != 0) {
        func_8006FA30(v, s->vout, q);
    }
}
