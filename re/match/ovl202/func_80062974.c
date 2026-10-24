#include "ac1/ac1.h"

typedef struct {                /* 0x0C model emitter point */
    SVECTOR v;                  /* +0x00 offset; pad = f2c << 2 */
    u16 flags;                  /* +0x08 low byte kind, 0x8000 */
    s16 uv;                     /* +0x0A index into the uv table row */
} EmitPt;

typedef struct {
    u8 unk0[8];
    s32 off;                    /* +0x08 */
    void *verts;                /* +0x0C */
    u8 unk10[0x14];
    EmitPt *emit;               /* +0x24 */
} ModelHdr;

extern ModelHdr *volatile D_801A5F30;
extern ModelHdr *D_801A5F34[];
extern DVECTOR *D_8008E4A0[][4];
extern void func_8001380C(EmitPt *, SVECTOR *, s32);
extern void func_80056B54(WorldStruct *, SVECTOR *, s32, s32, s32, DVECTOR *, s32);

/* spawn the effects of the up to four emitter points selected by flags
 * bits 24..27 of a world structure's model */
void func_80062974(WorldStruct *w, s32 bit)
{
    SVECTOR v[2];
    EmitPt *e;
    u32 fl;
    s32 mask;
    s32 i;
    s32 rot;
    s32 idx;
    ModelHdr *m;

    fl = *(u32 *)w->unk30;
    if (fl & 0x0F000000) {
        idx = w->mesh + 260;
        m = D_801A5F34[idx];
        rot = *(s16 *)((u8 *)w + 0x16);
        D_801A5F30 = (ModelHdr *)((u8 *)m + m->off);
        mask = fl >> 24;
        e = D_801A5F30->emit;
        mask &= 0xF;
        i = 4;
        do {
            if (mask & 1) {
                func_8001380C(e, v, rot);
                v[0].vx = w->org[0] + (v[0].vx >> 2);
                v[0].vy = w->org[1] + (v[0].vy >> 2);
                v[0].vz = w->org[2] + (v[0].vz >> 2);
                func_80056B54(w, v, e->flags & 0xFF, bit, (e->flags & 0x8000) ? 0xFFFF : -2,
                              D_8008E4A0[w->unk30[3] >> 4][e->uv], e->v.pad >> 2);
            }
            mask >>= 1;
            e++;
        } while (--i);
    }
}
