#include "ac1/ac1.h"
/* ovl202 0x8006FD6C - set the probe up in a world struct's mesh space: start / end
 * and the box, relative to its origin (x4) and turned back by its quarter turns */
typedef struct Scr {
    u8 unk00[0x44];
    SVECTOR box[2];             /* +0x44 */
    s32 p[3];                   /* +0x54 */
    s32 d[3];                   /* +0x60 */
    u8 unk6C[0x20];
    WorldStruct *ws;            /* +0x8C */
    u8 unk90[0x24];
    SVECTOR ts;                 /* +0xB4 */
    s32 tv[4];                  /* +0xBC */
} Scr;
void func_8006FC88(s32 *in, s32 *out, s32 q);
void func_8006FA30(SVECTOR *in, SVECTOR *out, s32 q);
void func_8006FB14(SVECTOR *b);

void func_8006FD6C(Scr *s, s32 *pts)
{
    s32 q;

    q = -s->ws->unk16 & 3;
    s->tv[0] = (pts[0] - s->ws->org[0]) * 4;
    s->tv[1] = (pts[1] - s->ws->org[1]) * 4;
    s->tv[2] = (pts[2] - s->ws->org[2]) * 4;
    func_8006FC88(s->tv, s->p, q);
    s->tv[0] = (pts[3] - s->ws->org[0]) * 4;
    s->tv[1] = (pts[4] - s->ws->org[1]) * 4;
    s->tv[2] = (pts[5] - s->ws->org[2]) * 4;
    func_8006FC88(s->tv, s->d, q);
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
