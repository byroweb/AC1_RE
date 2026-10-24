#include "ac1/ac1.h"
/* ovl202 0x8005C904 - draw (or queue) one world section if its box is on screen:
 * near sections (inside box + 1300) are lit and drawn at once (func_80067EBC /
 * func_80068D18); others are transformed through the quarter-turn camera tables and
 * either drawn (func_8005BE20 / func_80064684, when func_800674B0 says so) or queued
 * as a 24-byte entry on *near_list / *far_list. */

typedef struct {                /* 0x40, at 0x801D0B68 */
    u8 pad00[0x10];
    s16 x;                      /* +0x10 */
    s16 y;                      /* +0x12 */
    s16 z;                      /* +0x14 */
    s16 rot;                    /* +0x16 quarter turns, negated */
    SVECTOR min;                /* +0x18 (vy at +0x1A, vz +0x1C, pad +0x1E = model) */
    SVECTOR max;                /* +0x20 */
    u8 pad28[8];
    u32 flags;                  /* +0x30 low byte light set, 0x2000 margin test, >>28 */
    u32 vis;                    /* +0x34 bit 24 drawn; (>>8) & 0xFFF0 margin */
} Sect;

typedef struct {                /* 0x18 queued section */
    s32 t[3];                   /* +0x00 camera-space position */
    s16 rot;                    /* +0x0C */
    s16 size;                   /* +0x0E */
    Sect *s;                    /* +0x10 */
    void *ls;                   /* +0x14 */
} DrawEnt;

typedef struct {                /* 0x14 */
    s16 m[3][3];
    s16 pad;
} Rot20;

typedef struct {                /* 0x68 */
    Rot20 lm[4];                /* +0x00 light matrix per quarter turn */
    s16 color[3][3];            /* +0x50 */
    u8 pad62_[0];
    u8 r;                       /* +0x62 */
    u8 g;                       /* +0x63 */
    u8 b;                       /* +0x64 */
    u8 pad65;
    u16 fog;                    /* +0x66 */
} LightSet;

typedef struct {
    u8 pad00[4];
    u16 nv;                     /* +0x04 */
} PartRec;

typedef struct {
    u8 pad00[8];
    u32 nrec;                   /* +0x08 */
} ModelBody;

typedef struct {
    u8 pad00[8];
    s32 off;                    /* +0x08 */
} ModelHdr;

typedef struct {                /* 0x801A5F30 */
    ModelBody *model;           /* +0x0000 */
    ModelHdr *tbl[0x184];       /* +0x0004 (0x801A5F34) */
    void *verts;                /* +0x0614 (0x801A6544) */
    u8 pad0618[0x6A34 - 0x618];
    LightSet lights[1];         /* +0x6A34 (0x801AC964) */
    u8 pad6a9c[0x7734 - 0x6A9C];
    s32 fog;                    /* +0x7734 (0x801AD664) */
    u8 pad7738[0x14];
    MATRIX cam;                 /* +0x774C (0x801AD67C) */
    MATRIX cam2;                /* +0x776C */
    u8 pad778c[0x10];
    MATRIX far[4];              /* +0x779C (0x801AD6CC) */
    MATRIX near[4];             /* +0x781C (0x801AD74C) */
} RenderGlobals;

extern RenderGlobals D_801A5F30;
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void SetColorMatrix(void *m);
extern void SetLightMatrix(void *m);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);
extern VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);
extern s32 func_8005C710(SVECTOR *a, SVECTOR *b);
extern s32 func_800674B0(Sect *s, s32 nv);
extern void func_80067EBC(u16 k);
extern void func_80068D18(u16 k, u32 f);
extern void func_8005BE20(u16 k, s32 a);
extern void func_80064684(u16 k, s32 a);

void func_8005C904(Sect *s, s32 flag, DrawEnt **near_list, DrawEnt **far_list, SVECTOR *cam)
{
    VECTOR out;
    VECTOR d;
    s32 rot;
    LightSet *ls;
    s32 m;
    s32 k;
    s32 draw;
    ModelHdr *h;
    PartRec *rec;
    DrawEnt *e;
    u32 f;
    RenderGlobals *g;
    s32 t;

    SetRotMatrix(&D_801A5F30.cam);
    SetTransMatrix((MATRIX *)((u8 *)&D_801A5F30.cam + 12));
    if (func_8005C710(&s->min, &s->max) == 0) {
        return;
    }
    s->vis |= 0x1000000;
    ls = &D_801A5F30.lights[*(u8 *)&s->flags];
    rot = -s->rot & 3;
    d.vx = (s->x - cam->vx) << 2;
    d.vy = (s->y - cam->vy) << 2;
    d.vz = (s->z - cam->vz) << 2;
    if (flag == 0 && (s->flags & 0x2000)) {
        m = (s->vis >> 8) & 0xFFF0;
        if (!(s->min.vx - m < cam->vx && cam->vx < s->max.vx + m &&
              s->min.vy - m < cam->vy && cam->vy < s->max.vy + m &&
              s->min.vz - m < cam->vz && cam->vz < s->max.vz + m)) {
            flag = 1;
        }
    }
    g = &D_801A5F30;
    t = s->min.pad + 260;
    h = D_801A5F30.tbl[t];
    g->model = (ModelBody *)((u8 *)h + h->off);
    k = flag;
    if (D_801A5F30.model->nrec < 2) {
        k = 0;
    }
    rec = (PartRec *)((u8 *)D_801A5F30.model + (k * 32 + 12) - k * 4);
    D_801A5F30.verts = *(void **)rec;
    draw = func_800674B0(s, rec->nv);
    if (flag == 0 && s->min.vx - 1300 < cam->vx && cam->vx < s->max.vx + 1300 &&
        s->min.vy - 1300 < cam->vy && cam->vy < s->max.vy + 1300 &&
        s->min.vz - 1300 < cam->vz && cam->vz < s->max.vz + 1300) {
        SetColorMatrix(ls->color);
        SetBackColor(ls->r, ls->g, ls->b);
        SetLightMatrix(&ls->lm[rot]);
        {
            MATRIX *nm = g->near;
            ApplyMatrixLV(nm, &d, &out);
            SetRotMatrix(&nm[rot]);
        }
        SetTransMatrix((MATRIX *)((u8 *)&out - 20));
        if (draw) {
            func_80067EBC(k);
        } else {
            func_80068D18(k, s->flags >> 28);
        }
        return;
    }
    ApplyMatrixLV(D_801A5F30.far, &d, &out);
    if (flag == 0) {
        if (draw) {
            SetColorMatrix(ls->color);
            SetBackColor(ls->r, ls->g, ls->b);
            SetLightMatrix(&ls->lm[rot]);
            f = ls->fog;
            if (f != 0) {
                f *= 2;
                SetFogNearFar(f >> 5, f >> 3, 160);
            }
            D_801A5F30.fog = ls->fog;
            SetRotMatrix(&D_801A5F30.far[rot]);
            SetTransMatrix((MATRIX *)((u8 *)&out - 20));
            func_8005BE20(k, 120);
            return;
        }
        {
            DrawEnt *ne = *near_list;
            ne->t[0] = out.vx;
            ne->t[1] = out.vy;
            ne->t[2] = out.vz;
            ne->rot = rot;
            ne->ls = ls;
            ne->s = s;
            *near_list = ne + 1;
        }
        return;
    }
    if (draw) {
        SetColorMatrix(ls->color);
        SetBackColor(ls->r, ls->g, ls->b);
        SetLightMatrix(&ls->lm[rot]);
        SetRotMatrix(&D_801A5F30.far[rot]);
        f = ls->fog;
        if (f != 0) {
            f *= 2;
            SetFogNearFar(f >> 5, f >> 3, 160);
        }
        D_801A5F30.fog = ls->fog;
        SetTransMatrix((MATRIX *)((u8 *)&out - 20));
        func_80064684(k, 120);
        return;
    }
    e = *far_list;
    e->t[0] = out.vx;
    e->t[1] = out.vy;
    e->t[2] = out.vz;
    e->rot = rot;
    if (k != 0) {
        e->size = 40;
    } else {
        e->size = 12;
    }
    e->ls = ls;
    e->s = s;
    *far_list = ++e;
}
