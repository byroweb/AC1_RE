#include "ac1/ac1.h"

typedef struct {                /* 0x170 AC / enemy entity view */
    void *def;                  /* +0x000 */
    u8 pad004[4];
    SVECTOR pos;                /* +0x008 (pad = facing) */
    u8 mdl[6];                  /* +0x010 */
    u16 section;                /* +0x016 */
    u8 pad018[0x14];
    u16 y2;                     /* +0x02C second-section height */
    u8 pad02e[2];
    u16 sec2;                   /* +0x030 second section, -1 none */
    u16 flags;                  /* +0x032 */
    u8 pad034[0x2C];
    s16 *state;                 /* +0x060 */
    u8 mode;                    /* +0x064 */
    u8 m65;                     /* +0x065 */
    u8 pad066[4];
    u16 w;                      /* +0x06A */
    u16 h;                      /* +0x06C */
    s16 m6e;                    /* +0x06E */
    u8 pad070[2];
    s16 m72;                    /* +0x072 */
    s16 m74;                    /* +0x074 */
    s16 m76;                    /* +0x076 */
    u8 pad078[0xD0];
    u16 fire_flags;             /* +0x148 */
    u8 pad14a[0x12];
    u32 m15c;                   /* +0x15C */
    u8 pad160[0x10];
} DEnt;

typedef struct {                /* 0x34 world object */
    SVECTOR pos;                /* +0x00 (pad = model id, -1 none) */
    SVECTOR rot;                /* +0x08 (pad = world structure) */
    SVECTOR scl;                /* +0x10 */
    s8 zbias;                   /* +0x18 */
    u8 m19;                     /* +0x19 */
    s16 m1a;                    /* +0x1A */
    u16 flags;                  /* +0x1C */
    u8 pad1e[4];
    s8 bmin[3];                 /* +0x22 box around pos, units of 256 */
    s8 bmax[3];                 /* +0x25 */
    u8 m28;                     /* +0x28 */
    u8 m29[11];                 /* +0x29 */
} WObj;

typedef struct {                /* 0xE0 projectile / effect view */
    SVECTOR pos;                /* +0x00 (pad = dead) */
    u8 pad08[6];
    u16 ws;                     /* +0x0E */
    u8 type;                    /* +0x10 */
    u8 pad11[0xA7];
    SVECTOR b8;                 /* +0xB8 */
    u16 c0;                     /* +0xC0 */
    s16 c2;                     /* +0xC2 */
    u16 c4;                     /* +0xC4 */
    u16 c6;                     /* +0xC6 */
    u8 c8[4];                   /* +0xC8 */
    u8 cc[2];                   /* +0xCC */
    s16 ce;                     /* +0xCE */
    u8 pad0d0[0xC];
    u8 dc[4];                   /* +0xDC */
} PEnt;

typedef struct {                /* 0x68 */
    u8 pad00[0x68];
} WLight;

typedef struct {
    u32 mask[0x41];             /* +0x000 0x801AC860 */
    WLight lights[32];          /* +0x104 */
    u8 pad[0xE08 - 0x104 - 32 * 0x68];
    struct DCam *camobj;        /* +0xE08 0x801AD668 */
    u8 pade0c[0x10];
    MATRIX cam;                 /* +0xE1C 0x801AD67C (t at 0x801AD690) */
} DWorld;

typedef struct {                /* 12 */
    u8 spr[8];
    u32 col;
} SprDef;

typedef struct DCam {
    u8 pad[0x30];
    u32 flags;                  /* +0x30 */
    u32 m34;                    /* +0x34 */
} DCam;

typedef struct {                /* 0x18 */
    u8 pad[0x16];
    s16 kind;                   /* +0x16 */
} WObjDef;

#define VIS(m, s) ((m)[(s) >> 5] & (1 << ((s) & 0x1F)))

extern DCam *D_801AD668;
extern DEnt D_801A26B8[16];
extern WObj D_80198B08[512];
extern PEnt D_801AEB3C[256];
extern DWorld D_801AC860;
extern MATRIX D_801AD67C;
extern s32 D_801AC80C;
extern WObjDef D_801987F8[];
extern u8 D_80039C70[];
extern u8 D_8008E4D8[];
extern SprDef D_8008ED68[];
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern s32 func_80061F44(s32 w, void *pos, s32 a, s32 b);
extern s32 func_80054750(s16 **state, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s16 *g);
extern void func_8005DEC4(u16 model, SVECTOR *pos, u8 *mdl, s16 *state, u16 *ff, u32 f, s32 sec, s32 up,
                          s32 down, s32 w, s32 h);
extern void func_8005DC7C(s8 mode, u16 model, SVECTOR *pos, u8 *mdl, s16 *state, s32 up, s32 down);
extern void func_8005E434(WObj *o);
extern void func_8005EB50(s32 dy, s32 a, u8 *b);
extern void func_8005E300(u16 model, WObj *o, WLight *l);
extern void func_8005E22C(u16 model, SVECTOR *rot, WLight *l);
extern void func_8005D19C(u16 model, SVECTOR *pos, SVECTOR *rot, SVECTOR *scl, s32 a4, s32 a5, s32 a6,
                          s32 a7, s32 a8, u8 *a9, u8 *a10, s32 a11, s32 a12, s32 a13, s32 a14,
                          s32 a15, s32 a16);
extern void func_80062090(s32 w);
extern s32 func_8005C630(SVECTOR *v);
extern void func_8005FB90(SVECTOR *a, SVECTOR *b, SprDef *spr, u16 *col, s32 w);
extern void func_8006023C(SVECTOR *pos, SprDef *spr, u32 *col, s32 size, s32 z);
extern void func_80060800(SVECTOR *pos, u16 *rot, SVECTOR *uv, s32 s);
extern void func_8005DB10(s32 model, s32 part, SVECTOR *pos, SVECTOR *rot, u16 *scale, u8 *a5, s32 up,
                          s32 down);
extern void func_800604B8(SVECTOR *pos, SprDef *spr, s32 w, s32 h, s32 z);

/* draw the mission world: the player AC, the other entities, the world
 * objects, the lamps and the projectiles / effects, each culled against
 * the visible sections and the view */
void func_8006131C(void)
{
    SVECTOR v;
    DEnt *e;
    WObj *o;
    PEnt *p;
    s32 w;
    s32 i;
    s32 sec;
    s32 vis;
    u16 t;

    if (D_801AC860.camobj == 0) {
        return;
    }
    w = 0xFFFF;
    if (D_801AD668->flags & 0x4000) {
        w = (D_801AD668->m34 & 0xFFF) << 4;
    }
    e = D_801A26B8;
    if (e->def != 0) {
        if (!(D_801A26B8[0].flags & 0x8001)) {
            if ((D_801A26B8[0].flags & 0x4000) && D_801A26B8[0].state != 0) {
                *D_801A26B8[0].state = 2;
            } else {
                func_80054750(&e->state, e->pos.pad + 128, e->mode, e->m6e, e->m72, e->m65, e->m74, &e->m76);
            }
            sec = (s16)e->section;
            if (VIS(D_801AC860.mask, sec)) {
                SetRotMatrix(&D_801AC860.cam);
                SetTransMatrix((MATRIX *)((u8 *)&D_801AC860.cam + 12));
                if (func_80061F44(w, &e->pos, e->w, e->h) != 0) {
                    func_8005DEC4(e->pos.pad + 128, &e->pos, e->mdl, e->state, &e->fire_flags, e->m15c, sec,
                                  100, 120, e->w, e->h);
                    vis = 1;
                }
            }
            t = e->sec2;
            if ((s16)t != -1 && (D_801AC860.mask[(s16)t >> 5] & (1 << (t & 0x1F)))) {
                SetRotMatrix(&D_801AC860.cam);
                SetTransMatrix((MATRIX *)((u8 *)&D_801AC860.cam + 12));
                v.vy = e->y2;
                v.vx = e->pos.vx;
                v.vz = e->pos.vz;
                if (func_80061F44(w, &v, e->w, e->h >> 2) != 0) {
                    func_8005DC7C(vis, e->pos.pad + 128, &v, e->mdl, e->state, 100, 120);
                }
            }
        } else if ((D_801A26B8[0].flags & 0x4000) && D_801A26B8[0].state != 0) {
            *D_801A26B8[0].state = 2;
        }
    }
    e = &D_801A26B8[1];
    i = 15;
    do {
        if (e->def != 0) {
            if (!(e->flags & 0x8001)) {
                sec = (s16)e->section;
                vis = 0;
                if (VIS(D_801AC860.mask, sec)) {
                    SetRotMatrix(&D_801AC860.cam);
                    SetTransMatrix((MATRIX *)((u8 *)&D_801AC860.cam + 12));
                    if (func_80061F44((e->flags & 2) ? w * 2 : w, &e->pos, e->w, e->h) != 0) {
                        if ((e->flags & 0x4000) && e->state != 0) {
                            *e->state = 2;
                        } else if (func_80054750(&e->state, e->pos.pad + 128, e->mode, e->m6e, e->m72, e->m65,
                                                 e->m74, &e->m76) == 0) {
                            goto sec2;
                        }
                        func_8005DEC4(e->pos.pad + 128, &e->pos, e->mdl, e->state, &e->fire_flags, e->m15c,
                                      sec, 100, 120, e->w, e->h);
                        vis = 1;
                    }
                }
            sec2:
                t = e->sec2;
                if ((s16)t != -1 && (D_801AC860.mask[(s16)t >> 5] & (1 << (t & 0x1F)))) {
                    if (vis == 0) {
                        if ((e->flags & 0x4000) && e->state != 0) {
                            *e->state = 2;
                        } else if (func_80054750(&e->state, e->pos.pad + 128, e->mode, e->m6e, e->m72,
                                                 e->m65, e->m74, &e->m76) == 0) {
                            goto next;
                        }
                    }
                    SetRotMatrix(&D_801AD67C);
                    SetTransMatrix((MATRIX *)((u8 *)&D_801AD67C + 12));
                    v.vy = e->y2;
                    v.vx = e->pos.vx;
                    v.vz = e->pos.vz;
                    if (func_80061F44(w, &v, e->w, e->h >> 2) != 0) {
                        func_8005DC7C(vis, e->pos.pad + 128, &v, e->mdl, e->state, 100, 120);
                    }
                }
            } else if ((e->flags & 0x4000) && e->state != 0) {
                *e->state = 2;
            }
        }
    next:
        e++;
    } while (--i);
    o = D_80198B08;
    i = 512;
    do {
        if (o->pos.pad == -1) {
            goto onext;
        }
        if (o->flags & 0x8000) {
            goto onext;
        }
        if (o->flags & 1) {
            if (D_801AC80C != 0 && D_801987F8[o->pos.pad].kind == 244) {
                func_8005E434(o);
                goto onext;
            }
            if (D_801AC860.cam.t[0] < o->pos.vx + (o->bmin[0] << 8)) {
                goto onext;
            }
            if (o->pos.vx + (o->bmax[0] << 8) < D_801AC860.cam.t[0]) {
                goto onext;
            }
            if (D_801AC860.cam.t[1] < o->pos.vy + (o->bmin[1] << 8)) {
                goto onext;
            }
            if (o->pos.vy + (o->bmax[1] << 8) < D_801AC860.cam.t[1]) {
                goto onext;
            }
            if (D_801AC860.cam.t[2] < o->pos.vz + (o->bmin[2] << 8)) {
                goto onext;
            }
            if (o->pos.vz + (o->bmax[2] << 8) < D_801AC860.cam.t[2]) {
                goto onext;
            }
            if (o->flags & 2) {
                func_8005EB50(o->pos.vy - D_801AC860.cam.t[1], o->m28, o->m29);
            } else if (o->flags & 8) {
                func_8005E300(o->pos.pad + 160, o,
                              &D_801AC860.lights[g_world_structs[o->rot.pad].unk30[0]]);
            } else if (o->flags & 0x20) {
                sec = o->rot.pad;
                goto draw;
            } else {
                func_8005E22C(o->pos.pad + 160, &o->rot,
                              &D_801AC860.lights[g_world_structs[o->rot.pad].unk30[0]]);
            }
        } else {
            sec = o->rot.pad;
            if (VIS(D_801AC860.mask, sec)) {
                SetRotMatrix(&D_801AC860.cam);
                SetTransMatrix((MATRIX *)((u8 *)&D_801AC860.cam + 12));
                if (func_80061F44(w, o, 500, 500) != 0) {
                draw:
                    func_8005D19C(o->pos.pad + 160, &o->pos, &o->rot, &o->scl, o->m19, o->m1a, 0, 255, 0,
                                  D_80039C70, D_8008E4D8, 0, sec, o->zbias + 100, o->zbias + 120, 0, 0);
                }
            }
        }
    onext:
        o++;
    } while (--i);
    func_80062090(w);
    w *= 2;
    p = D_801AEB3C;
    i = 256;
    do {
        if (p->pos.pad == 0) {
            sec = (s16)p->ws;
            if (VIS(D_801AC860.mask, sec)) {
                SetRotMatrix(&D_801AC860.cam);
                SetTransMatrix((MATRIX *)((u8 *)&D_801AC860.cam + 12));
                switch (p->type) {
                case 0:
                    if (func_80061F44(w, p, 500, 500) != 0) {
                        func_8005D19C(p->c0, &p->pos, &p->b8, (SVECTOR *)p->dc, p->c2, (s16)p->c4, 0, 255, 0,
                                      D_80039C70, p->c8, p->c6, sec, 100, 120, 0, 0);
                    }
                    break;
                case 1:
                    if (!(func_8005C630(&p->pos) & func_8005C630(&p->b8))) {
                        func_8005FB90(&p->pos, &p->b8, &D_8008ED68[p->c4], &p->c0, (s16)p->c6);
                    }
                    break;
                case 2:
                    if (func_80061F44(w, p, p->b8.vy, p->b8.vy) != 0) {
                        func_8006023C(&p->pos, &D_8008ED68[(u16)p->b8.vx], &D_8008ED68[(u16)p->b8.vx].col,
                                      p->b8.vy, p->b8.vz);
                    }
                    break;
                case 3:
                    if (func_80061F44(w, p, p->ce, p->ce) != 0) {
                        func_80060800(&p->pos, &p->c6, &p->b8, p->ce);
                    }
                    break;
                case 4:
                    if (func_80061F44(w, p, 500, 500) != 0) {
                        func_8005DB10((u16)p->b8.vx, (u16)p->b8.vy, &p->pos, (SVECTOR *)&p->b8.vz, &p->c4,
                                      p->cc, 100, 120);
                    }
                    break;
                case 5:
                    if (func_80061F44(w, p, p->b8.vy, p->b8.vz) != 0) {
                        func_800604B8(&p->pos, &D_8008ED68[(u16)p->b8.vx], p->b8.vy, p->b8.vz, p->b8.pad);
                    }
                    break;
                case 6:
                    if (func_80061F44(w, p, p->b8.pad, p->b8.pad) != 0) {
                        func_8006023C(&p->pos, &D_8008ED68[(u16)p->b8.vz], (u32 *)&p->b8, p->b8.pad,
                                      (s16)p->c0);
                    }
                    break;
                }
            }
        }
        p++;
    } while (--i);
}
