#include "ac1/ac1.h"
/* ovl202 0x80071904 - func_8006FD6C for the probe record: segment ends (SVECTOR) and
 * box into a world struct's mesh space, then func_800718B4 orders the ends */
typedef struct Probe {
    u8 unk00[0x5C];
    s32 min[3];                 /* +0x5C */
    s32 max[3];                 /* +0x68 */
    SVECTOR box[2];             /* +0x74 */
    u8 unk84[0xC];
    WorldStruct *ws;            /* +0x90 */
    u8 unk94[0x50];
    SVECTOR ts;                 /* +0xE4 */
    s32 tv[4];                  /* +0xEC */
} Probe;
void func_8006FC88(s32 *in, s32 *out, s32 q);
void func_8006FA30(SVECTOR *in, SVECTOR *out, s32 q);
void func_8006FB14(SVECTOR *b);
void func_800718B4(s32 *minmax);

void func_80071904(Probe *s, SVECTOR *pts)
{
    s32 q;

    q = -s->ws->unk16 & 3;
    s->tv[0] = (pts[0].vx - s->ws->org[0]) * 4;
    s->tv[1] = (pts[0].vy - s->ws->org[1]) * 4;
    s->tv[2] = (pts[0].vz - s->ws->org[2]) * 4;
    func_8006FC88(s->tv, s->min, q);
    s->tv[0] = (pts[1].vx - s->ws->org[0]) * 4;
    s->tv[1] = (pts[1].vy - s->ws->org[1]) * 4;
    s->tv[2] = (pts[1].vz - s->ws->org[2]) * 4;
    func_8006FC88(s->tv, s->max, q);
    func_800718B4(s->min);
    s->ts.vx = (s->ws->bbox_min[0] - s->ws->org[0]) << 2;
    s->ts.vy = (s->ws->bbox_min[1] - s->ws->org[1]) << 2;
    s->ts.vz = (s->ws->bbox_min[2] - s->ws->org[2]) << 2;
    func_8006FA30(&s->ts, &s->box[0], q);
    s->ts.vx = (s->ws->bbox_max[0] - s->ws->org[0]) << 2;
    s->ts.vy = (s->ws->bbox_max[1] - s->ws->org[1]) << 2;
    s->ts.vz = (s->ws->bbox_max[2] - s->ws->org[2]) << 2;
    func_8006FA30(&s->ts, &s->box[1], q);
    func_8006FB14(s->box);
}
