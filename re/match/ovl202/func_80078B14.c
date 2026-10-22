#include "ac1/ac1.h"

typedef struct {
    s16 v[5];
} Box10;

typedef struct {
    u16 h0;             /* +0x00 */
    u16 h2;             /* +0x02 */
    u16 h4;             /* +0x04 */
    u8 pad06[0x22];
    s32 w28;            /* +0x28 */
} AcGeom;

typedef struct {
    u8 state;           /* +0x00 */
    u8 slot;            /* +0x01 */
    u16 ap;             /* +0x02 */
    u16 x;              /* +0x04 */
    u16 y;              /* +0x06 */
    u16 z;              /* +0x08 */
    s16 geom;           /* +0x0A */
    SVECTOR rot;        /* +0x0C */
    u16 flags;          /* +0x14 */
    u8 pad16[4];
    u16 ap_max;         /* +0x1A */
    u8 pad1c[6];
    Box10 box;          /* +0x22 */
} AcSpawn;

typedef struct {
    AcSpawn *def;       /* +0x00 */
    AcGeom *geom;       /* +0x04 */
    s16 x;              /* +0x08 */
    s16 y;              /* +0x0A */
    s16 z;              /* +0x0C */
    s16 w;              /* +0x0E */
    SVECTOR rot;        /* +0x10 */
    u8 pad18[6];
    s16 h1e;            /* +0x1E */
    u8 pad20[8];
    s16 h28;            /* +0x28 */
    s16 h2a;            /* +0x2A */
    s16 h2c;            /* +0x2C */
    s16 h2e;            /* +0x2E */
    s16 h30;            /* +0x30 */
    s16 h32;            /* +0x32 */
    u8 pad34[4];
    s16 h38;            /* +0x38 */
    u8 b3a;             /* +0x3A */
    u8 pad3b;
    u8 b3c;             /* +0x3C */
    u8 b3d;             /* +0x3D */
    u8 b3e;             /* +0x3E */
    u8 b3f;             /* +0x3F */
    u8 b40;             /* +0x40 */
    u8 b41;             /* +0x41 */
    u8 pad42[6];
    void *target;       /* +0x48 */
    void *fn[5];        /* +0x4C */
    s32 w60;            /* +0x60 */
    u8 b64;             /* +0x64 */
    u8 b65;             /* +0x65 */
    u8 pad66[2];
    u8 b68;             /* +0x68 */
    u8 pad69;
    s16 h6a;            /* +0x6A */
    s16 h6c;            /* +0x6C */
    s16 h6e;            /* +0x6E */
    u8 pad70[2];
    s16 h72;            /* +0x72 */
    u8 pad74[2];
    SVECTOR v76;        /* +0x76 */
    u8 pad7e[2];
    s32 w80;            /* +0x80 */
    u8 pad84[0x78];
    Box10 box;          /* +0xFC */
    u8 pad106[0x42];
    s16 h148;           /* +0x148 */
    u8 pad14a[0x12];
    s32 w15c;           /* +0x15C */
    s16 h160;           /* +0x160 */
    s16 h162;           /* +0x162 */
    s16 h164;           /* +0x164 */
    u8 pad166[0x0A];
} AcInit;

typedef struct {
    AcGeom geoms[288];      /* 0x8019F538 */
    AcInit ac[16];          /* 0x801A26B8 */
} AcWorld;

extern AcWorld D_8019F538;
extern SVECTOR D_80039C78;

void func_80078B14(AcInit *e, AcSpawn *d)
{
    AcGeom *g;

    g = &D_8019F538.geoms[d->geom];
    d->state = 1;
    d->slot = e - D_8019F538.ac;
    e->def = d;
    e->geom = g;
    e->b3a = 3;
    e->h38 = 0;
    e->h162 = e->h164 = d->ap_max;
    e->h160 = d->ap;
    e->h28 = e->h6a = g->h0;
    e->h2a = e->h6c = g->h2;
    e->h1e = g->h4;
    e->w80 = g->w28;
    e->h32 = (d->flags >> 4) & 2;
    e->x = d->x;
    e->y = d->y - 1;
    e->z = d->z;
    e->w = d->geom;
    e->box = d->box;
    e->rot = d->rot;
    e->h2e = 0x7FFF;
    e->target = 0;
    e->b64 = 0;
    e->h6e = 0;
    e->b65 = 255;
    e->b68 = 4;
    e->h2c = e->y;
    e->h30 = *(s16 *)((u8 *)e + 0x16);
    e->v76 = D_80039C78;
    e->v76.pad = -1;
    e->h72 = 0;
    e->w60 = 0;
    e->h148 = 0;
    e->w15c = 0;
    e->b3e = 0;
    e->b3f = 0;
    e->b40 = 255;
    e->b3d = 0;
    e->b3c = 0;
    e->b41 = 0;
    e->fn[0] = 0;
    e->fn[1] = 0;
    e->fn[2] = 0;
    e->fn[3] = 0;
    e->fn[4] = 0;
}
