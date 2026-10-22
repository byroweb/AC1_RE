#include "ac1/ac1.h"

typedef struct {
    u8 unk0[0x30];
    u32 flags;                  /* +0x30 bit 0x4000: LOD mask in +0x34 */
    u32 lod;                    /* +0x34 */
} CurWs;

typedef struct {
    u8 lm[0x50];
    s16 col[9];
    u8 r, g, b;
    u8 pad65;
    u16 fog;
} LightSet;                     /* 0x68 */

typedef struct {
    u8 unk0[0x30];
    u8 light;                   /* +0x30 */
    u8 unk31[0x0F];
} WStruct;

typedef struct {
    u32 vis[16];                /* +0x000 (0x801B4158) */
    u8 unk40[0x104 - 0x40];
    LightSet light[0x20];       /* +0x104 (0x801B425C) */
    u8 unke04[0xE30 - 0xE04];
    CurWs *cur;                 /* +0xE30 (0x801B4F88) current world structure */
    u8 unke34[0xE44 - 0xE34];
    MATRIX cam;                 /* +0xE44 (0x801B4F9C) */
} SceneG;

typedef struct {
    s16 vx, vy, vz;
    s16 section;                /* +0x16 of the entity: world-section index */
} RotSec;

typedef struct {
    void *def;                  /* +0x000 */
    u8 unk4[4];
    s16 x;                      /* +0x008 */
    s16 y;                      /* +0x00A */
    s16 z;                      /* +0x00C */
    u16 model;                  /* +0x00E */
    RotSec rot;                 /* +0x010 */
    u8 unk18[0x2C - 0x18];
    s16 shadow_y;               /* +0x02C */
    u8 unk2e[2];
    s16 shadow_ws;              /* +0x030 */
    u8 unk32[0x60 - 0x32];
    s32 obj;                    /* +0x060 */
    u8 unk64[0x6A - 0x64];
    u16 lw;                     /* +0x06A */
    u16 lh;                     /* +0x06C */
    u8 unk6e[0x148 - 0x6E];
    u16 fire_flags;             /* +0x148 */
    u8 unk14a[0x15C - 0x14A];
    u32 flags;                  /* +0x15C */
    u8 unk160[0x170 - 0x160];
} VsEnt;

typedef struct {
    s16 x, y, z;                /* +0x00 */
    s16 type;                   /* +0x06 -1 free */
    RotSec rot;                 /* +0x08 (section +0x0E) */
    s32 obj;                    /* +0x10 */
    u8 unk14[4];
    s8 lod;                     /* +0x18 */
    u8 part;                    /* +0x19 */
    s16 a5;                     /* +0x1A */
    u16 flags;                  /* +0x1C */
    u8 unk1e[4];
    s8 bmin[3];                 /* +0x22 trigger box (x 256) */
    s8 bmax[3];                 /* +0x25 */
    u8 sky;                     /* +0x28 */
    u8 skycol[11];              /* +0x29 */
} WObj;                         /* 0x34 */

typedef struct {
    SprInfo s;
    u32 rgb;
} SprRgb;                       /* 0x0C */

typedef struct {
    s16 x, y, z;                /* +0x00 */
    s16 free;                   /* +0x06 */
    u8 unk8[6];
    s16 ws;                     /* +0x0E */
    u8 kind;                    /* +0x10 */
    u8 unk11[0xB8 - 0x11];
    s16 a[20];                  /* +0xB8 per-kind draw parameters */
} FxObj;                        /* 0xE0 */

extern CurWs *volatile D_801B4F88;
extern SceneG D_801B4158;
extern VsEnt D_801AB748[2];
extern WObj D_801A4C68[512];
extern FxObj D_801B645C[256];
extern SprRgb D_8008A884[];
extern WStruct D_801D4488[];
extern s32 D_80039C70;
extern u16 D_80089F44;
extern s32 func_80060B08(s32 lod, void *pos, s32 w, s32 h);
extern void SetRotMatrix(void *m);
extern void SetTransMatrix(void *m);
extern void func_8005D068(u16 model, void *pos, void *rot, s32 o, void *flag, s32 b, s32 ws, s32 up,
                          s32 down, s32 c, s32 e);
extern void func_8005CE20(s8 mode, u16 model, void *pos, void *rot, s32 obj, s32 up, s32 down);
extern void func_8005DAFC(s32 roll, s32 fl, void *src);
extern void func_8005D47C(u16 model, void *pos, LightSet *lt);
extern void func_8005D3A8(u16 model, void *rot, LightSet *lt);
extern s32 func_8005C340(u16 model, void *pos, void *rot, void *o, s32 part, s32 a5, s32 a6, s32 a7,
                         s32 a8, void *a9, void *flag, s32 mask, s32 ws, s32 up, s32 down, s32 c, s32 e);
extern void func_80060C54(s32 lod);
extern s32 func_8005B7D4(void *v);
extern void func_8005EB3C(void *a, void *b, void *t, void *rgb, s32 width);
extern void func_8005F1E8(void *v, void *s, void *rgb, s32 size, s32 z);
extern void func_8005F7AC(void *v, void *rot, void *t, s32 s);
extern void func_8005CCB4(s32 model, s32 part, void *pos, void *rot, void *scale, void *a5, s32 up, s32 down);
extern void func_8005F464(void *v, void *t, s32 w, s32 h, s32 z);

/* 2-player scene: both ACs (with ground shadow), world objects, billboards,
 * then the 256 effect slots by kind */
void func_800602C8(void)
{
    VsEnt *e;
    WObj *o;
    FxObj *p;
    SVECTOR v;
    s32 lod;
    s32 i;
    s32 ws;
    s32 drawn;
    s32 sw;
    CurWs *cw;
    u16 fl;

    if (D_801B4F88 == 0) {
        return;
    }
    cw = D_801B4F88;
    lod = 0xFFFF;
    if (cw->flags & 0x4000) {
        lod = (cw->lod & 0xFFF) << 4;
    }
    e = D_801AB748;
    for (i = 2; i != 0; e++, i--) {
        if (e->def == 0) {
            continue;
        }
        ws = e->rot.section;
        drawn = 0;
        if (D_801B4158.vis[ws >> 5] & (1 << (ws & 0x1F))) {
            SetRotMatrix(&D_801B4158.cam);
            SetTransMatrix((u8 *)&D_801B4158.cam + 12);
            if (func_80060B08(lod, &e->x, e->lw, e->lh) != 0) {
                func_8005D068(e->model + 128, &e->x, &e->rot, e->obj, &e->fire_flags, e->flags, ws,
                              100, 120, e->lw, e->lh);
                drawn = 1;
            }
        }
        if (e->shadow_ws == -1) {
            continue;
        }
        if (!(D_801B4158.vis[e->shadow_ws >> 5] & (1 << (e->shadow_ws & 0x1F)))) {
            continue;
        }
        SetRotMatrix(&D_801B4158.cam);
        SetTransMatrix((u8 *)&D_801B4158.cam + 12);
        v.vy = e->shadow_y;
        v.vx = e->x;
        v.vz = e->z;
        if (func_80060B08(lod, &v, e->lw, e->lh >> 2) != 0) {
            func_8005CE20(drawn, e->model + 128, &v, &e->rot, e->obj, 100, 120);
        }
    }
    o = D_801A4C68;
    for (i = 512; i != 0; o++, i--) {
        if (o->type == -1) {
            continue;
        }
        fl = o->flags;
        if (fl & 0x8000) {
            continue;
        }
        if (fl & 1) {
            if (D_801B4158.cam.t[0] < o->x + (o->bmin[0] << 8)) {
                continue;
            }
            if (o->x + (o->bmax[0] << 8) < D_801B4158.cam.t[0]) {
                continue;
            }
            if (D_801B4158.cam.t[1] < o->y + (o->bmin[1] << 8)) {
                continue;
            }
            if (o->y + (o->bmax[1] << 8) < D_801B4158.cam.t[1]) {
                continue;
            }
            if (D_801B4158.cam.t[2] < o->z + (o->bmin[2] << 8)) {
                continue;
            }
            if (o->z + (o->bmax[2] << 8) < D_801B4158.cam.t[2]) {
                continue;
            }
            if (fl & 2) {
                func_8005DAFC(o->y - D_801B4158.cam.t[1], o->sky, o->skycol);
                continue;
            }
            if (fl & 8) {
                func_8005D47C(o->type + 160, o, &D_801B4158.light[D_801D4488[o->rot.section].light]);
                continue;
            }
            if (fl & 0x20) {
                ws = o->rot.section;
                goto draw;
            }
            func_8005D3A8(o->type + 160, &o->rot, &D_801B4158.light[D_801D4488[o->rot.section].light]);
            continue;
        }
        ws = o->rot.section;
        if (!(D_801B4158.vis[ws >> 5] & (1 << (ws & 0x1F)))) {
            continue;
        }
        SetRotMatrix(&D_801B4158.cam);
        SetTransMatrix((u8 *)&D_801B4158.cam + 12);
        if (func_80060B08(4000, o, 500, 500) == 0) {
            continue;
        }
    draw:
        func_8005C340(o->type + 160, o, &o->rot, &o->obj, o->part, o->a5, 0, 255, 0, &D_80039C70,
                      &D_80089F44, 0, ws, o->lod + 100, o->lod + 120, 0, 0);
    }
    func_80060C54(lod);
    lod <<= 1;
    p = D_801B645C;
    for (i = 256; i != 0; p++, i--) {
        if (p->free != 0) {
            continue;
        }
        ws = p->ws;
        if (!(D_801B4158.vis[ws >> 5] & (1 << (ws & 0x1F)))) {
            continue;
        }
        SetRotMatrix(&D_801B4158.cam);
        SetTransMatrix((u8 *)&D_801B4158.cam + 12);
        switch (p->kind) {
        case 0:
            if (func_80060B08(lod, p, 500, 500) != 0) {
                func_8005C340((u16)p->a[4], p, &p->a[0], &p->a[18], p->a[5], p->a[6], 0, 255, 0,
                              &D_80039C70, &p->a[8], (u16)p->a[7], ws, 100, 120, 0, 0);
            }
            break;
        case 1:
            if ((func_8005B7D4(p) & func_8005B7D4(&p->a[0])) == 0) {
                func_8005EB3C(p, &p->a[0], &D_8008A884[(u16)p->a[6]], &p->a[4], p->a[7]);
            }
            break;
        case 2:
            if (func_80060B08(lod, p, p->a[1], p->a[1]) != 0) {
                func_8005F1E8(p, &D_8008A884[(u16)p->a[0]].s, &D_8008A884[(u16)p->a[0]].rgb, p->a[1],
                              p->a[2]);
            }
            break;
        case 3:
            if (func_80060B08(lod, p, p->a[11], p->a[11]) != 0) {
                func_8005F7AC(p, &p->a[7], &p->a[0], p->a[11]);
            }
            break;
        case 4:
            if (func_80060B08(lod, p, 500, 500) != 0) {
                func_8005CCB4((u16)p->a[0], (u16)p->a[1], p, &p->a[2], &p->a[6], &p->a[10], 100, 120);
            }
            break;
        case 5:
            if (func_80060B08(lod, p, p->a[1], p->a[2]) != 0) {
                func_8005F464(p, &D_8008A884[(u16)p->a[0]], p->a[1], p->a[2], p->a[3]);
            }
            break;
        case 6:
            if (func_80060B08(lod, p, p->a[3], p->a[3]) != 0) {
                func_8005F1E8(p, &D_8008A884[(u16)p->a[2]], &p->a[0], p->a[3], p->a[4]);
            }
            break;
        }
    }
}
