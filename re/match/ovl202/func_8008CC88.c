/* ovl202 0x8008CC88  size 1536: world-object mesh renderer (flat lit F3/F4) */
#include "ac1/ac1.h"
#include "ac1/gte.h"

extern s32 D_801A5F2C;

typedef struct {                /* 8: one transformed vertex */
    s16 x, y;                   /* +0 SXY2 */
    s16 z;                      /* +4 SZ3 >> 2 (stored as a word) */
    u16 clip;                   /* +6 outcode, 0xFF = behind */
} WVtx;

typedef struct {                /* 0x1C: model table entry */
    SVECTOR *verts;             /* +0x00 */
    u16 nverts;                 /* +0x04 */
    u16 pad06;
    u8 *normals;                /* +0x08 */
    u8 pad0C[4];
    u32 *prims;                 /* +0x10 */
    u16 nprims;                 /* +0x14 */
    u16 pad16;
    u8 pad18[4];
} WModel;

typedef struct {                /* scratchpad 0x1F800000 */
    MATRIX m0;                  /* +0x000 */
    MATRIX rot[4];              /* +0x020 */
    MATRIX light[4];            /* +0x0A0 */
    SVECTOR cam;                /* +0x120 */
    SVECTOR pos;                /* +0x128 */
    VECTOR wpos;                /* +0x130 = t[] of the MATRIX at +0x11C */
    u32 *ot;                    /* +0x140 */
    u8 *prim;                   /* +0x144 */
    WModel *models;             /* +0x148 */
    union { u32 w; u8 b[4]; } cur; /* +0x14C */
    u8 *normals;                /* +0x150 */
    WVtx vtx[1];                /* +0x154 */
} WSpad;

typedef struct {                /* 0x40: view of WorldStruct */
    u8 pad00[6];
    s16 id;                     /* +0x06 -1 = unused */
    u8 pad08[8];
    SVECTOR org;                /* +0x10 */
    u8 pad18[6];
    s16 mesh;                   /* +0x1E */
    u8 pad20[0x10];
    s32 flags30;                /* +0x30 */
    s32 flags34;                /* +0x34 */
    u8 pad38[8];
} WObj;

#define WS ((WObj *)g_world_structs)

#define MDL(x) ((WModel *)((u8 *)sp->models + ((x) << 5) - ((x) << 2)))

void func_8008CC88(void) {
    s32 n;
    WObj *o;
    WVtx *w;
    s32 k;
    s32 cnt;
    s32 nv;
    WModel *mdl;
    u32 *pp;
    P_TAG *prim;
    WVtx *a, *b, *c, *d;
    s32 code;
    s32 z;
    s32 m;
    WSpad *sp = (WSpad *)0x1F800000;

    n = g_world_struct_count;
    sp->models = (WModel *)(D_801A5F2C + 0xC);
    o = WS;
    do {
        if (o->id != -1 && ((o->flags30 & 0x10000) || (o->flags34 & 0x01000000))
            && (m = o->mesh, MDL(m)->nverts != 0)) {
            gte_SetRotMatrix(&sp->m0);
            gte_SetTransMatrix(&sp->m0);
            sp->pos = o->org;
            sp->pos.vx = (sp->pos.vx >> 2) - sp->cam.vx;
            sp->pos.vy = (sp->pos.vy >> 2) - sp->cam.vy;
            sp->pos.vz = (sp->pos.vz >> 2) - sp->cam.vz;
            gte_ldv0(&sp->pos);
            gte_rtv0tr();
            gte_stlvnl(&sp->wpos);
            k = -sp->pos.pad & 3;
            gte_SetRotMatrix(&sp->rot[k]);
            gte_SetLightMatrix(&sp->light[k]);
            gte_SetTransMatrix((MATRIX *)((u8 *)&sp->wpos - 20));
            m = o->mesh;
            mdl = MDL(m);
            {
                SVECTOR *v = mdl->verts;
                nv = mdl->nverts;
                w = sp->vtx;
                do {
                    gte_ldv0(v);
                    gte_rtps();
                    v++;
                    gte_stsxy(&w->x);
                    gte_stszotz(&w->z);
                    if (w->z == 0) {
                        w->clip = 0xFF;
                    } else {
                        if (w->x < 0) {
                            w->clip = 1;
                        } else if (w->x >= 320) {
                            w->clip = 2;
                        } else {
                            w->clip = 0;
                        }
                        if (w->y < 0) {
                            w->clip |= 4;
                        } else if (w->y >= 240) {
                            w->clip |= 8;
                        }
                    }
                    w++;
                } while (--nv != 0);
            }
            pp = mdl->prims;
            cnt = mdl->nprims;
            sp->normals = mdl->normals;
            do {
                sp->cur.w = *pp;
                code = sp->cur.b[3] & 0xFC;
                pp++;
                switch (code) {
                case 0x28:
                    a = (WVtx *)((u8 *)sp->vtx + (((u16 *)pp)[3] >> 1));
                    b = (WVtx *)((u8 *)sp->vtx + (((u16 *)pp)[4] >> 1));
                    c = (WVtx *)((u8 *)sp->vtx + (((u16 *)pp)[5] >> 1));
                    d = (WVtx *)((u8 *)sp->vtx + (((u16 *)pp)[6] >> 1));
                    if ((s16)(d->clip | (c->clip | (a->clip | b->clip))) != 0xFF
                        && !(d->clip & (c->clip & (a->clip & b->clip)))) {
                        prim = (P_TAG *)sp->prim;
                        sp->prim = (u8 *)prim + 24;
                        ((u32 *)prim)[2] = *(u32 *)a;
                        ((u32 *)prim)[3] = *(u32 *)b;
                        ((u32 *)prim)[4] = *(u32 *)c;
                        ((u32 *)prim)[5] = *(u32 *)d;
                        gte_ldv0(sp->normals + ((u16 *)pp)[2]);
                        gte_ldrgb(pp);
                        gte_nccs();
                        gte_strgb(&prim->r0);
                        setlen(prim, 5);
                        z = (a->z + b->z + c->z + d->z) >> 2;
                        addPrim(&sp->ot[z], prim);
                    }
                    break;
                case 0x20:
                    a = (WVtx *)((u8 *)sp->vtx + (((u16 *)pp)[3] >> 1));
                    b = (WVtx *)((u8 *)sp->vtx + (((u16 *)pp)[4] >> 1));
                    c = (WVtx *)((u8 *)sp->vtx + (((u16 *)pp)[5] >> 1));
                    if ((s16)(c->clip | (a->clip | b->clip)) != 0xFF
                        && !(c->clip & (a->clip & b->clip))) {
                        prim = (P_TAG *)sp->prim;
                        sp->prim = (u8 *)prim + 20;
                        ((u32 *)prim)[2] = *(u32 *)a;
                        ((u32 *)prim)[3] = *(u32 *)b;
                        ((u32 *)prim)[4] = *(u32 *)c;
                        gte_ldv0(sp->normals + ((u16 *)pp)[2]);
                        gte_ldrgb(pp);
                        gte_nccs();
                        gte_strgb(&prim->r0);
                        setlen(prim, 4);
                        z = (a->z + b->z + c->z) / 3;
                        addPrim(&sp->ot[z], prim);
                    }
                    break;
                }
                pp += sp->cur.b[1];
            } while (--cnt != 0);
        }
        o++;
    } while (--n != 0);
}
