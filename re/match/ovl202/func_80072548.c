#include "ac1/ac1.h"

typedef struct {
    u8 pad[8];
    SVECTOR d;          /* +0x08 per-frame displacement */
} WsMover;

typedef struct {
    CollBox box;        /* +0x00 */
    SVECTOR org;        /* +0x10 */
    CollBox bbox;       /* +0x18 */
    u8 pad28[4];
    WsMover *mv;        /* +0x2C */
    u8 pad30[0x10];
} MovWs;

typedef struct {
    void *def;
    u8 pad04[4];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x18];
    u16 r;              /* +0x28 */
    u16 h;              /* +0x2A */
    u8 pad2c[0x144];
} PushEnt;

extern PushEnt D_801A26B8[];
extern void func_8006DD00(MovWs *ws, s32 idx);
extern void func_8006DC3C(CollBox *b, s32 idx);
extern s32 func_800722B4(MovWs *ws, SVECTOR *seg, SVECTOR *v, s32 mode);
extern s32 func_800749A0(PushEnt *e, AcPos *pos, SVECTOR *out, SVECTOR *v);

void func_80072548(MovWs *ws)
{
    SVECTOR org0;
    SVECTOR org1;
    CollBox b0;
    CollBox b1;
    SVECTOR seg[2];
    SVECTOR out;
    SVECTOR d;
    WsMover *mv;
    PushEnt *e;
    s32 idx;
    s32 i;

    idx = ws - (MovWs *)g_world_structs;
    mv = ws->mv;
    func_8006DD00(ws, idx);
    org0 = ws->org;
    org1 = org0;
    org1.vx += mv->d.vx;
    org1.vy += mv->d.vy;
    org1.vz += mv->d.vz;
    b0 = ws->bbox;
    b1 = b0;
    b1.min.vx += mv->d.vx;
    b1.max.vx += mv->d.vx;
    b1.min.vy += mv->d.vy;
    b1.max.vy += mv->d.vy;
    b1.min.vz += mv->d.vz;
    b1.max.vz += mv->d.vz;
    e = D_801A26B8;
    for (i = 16; i != 0; e++, i--) {
        if (e->def == 0) {
            continue;
        }
        if (e->r == 0) {
            continue;
        }
        if (*(u16 *)((u8 *)e->def + 0x14) & 4) {
            continue;
        }
        seg[0].vx = e->pos.x - e->r;
        seg[1].vx = e->pos.x + e->r;
        seg[0].vz = e->pos.z - e->r;
        seg[1].vz = e->pos.z + e->r;
        seg[0].vy = e->pos.y - e->h;
        seg[1].vy = e->pos.y + 24;
        ws->org = org0;
        ws->bbox = b0;
        if (func_800722B4(ws, seg, 0, 0) == 0) {
            goto retry;
        }
    hit:
        d = mv->d;
        func_800749A0(e, &e->pos, &out, &d);
        e->pos.x = out.vx;
        e->pos.y = out.vy;
        e->pos.z = out.vz;
        continue;
    retry:
        ws->org = org1;
        ws->bbox = b1;
        if (func_800722B4(ws, seg, 0, 0) != 0) {
            goto hit;
        }
    }
    ws->org = org1;
    ws->bbox = b1;
    ws->box.min.vx += mv->d.vx;
    ws->box.min.vy += mv->d.vy;
    ws->box.min.vz += mv->d.vz;
    ws->box.max.vx += mv->d.vx;
    ws->box.max.vy += mv->d.vy;
    ws->box.max.vz += mv->d.vz;
    func_8006DC3C(&ws->box, idx);
}
