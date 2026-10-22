#include "ac1/ac1.h"

/* Actor definition, 0x2C each: four at D_8019B4A0, right before the actors. */
typedef struct {
    s16 w;                      /* +0x00 */
    s16 h;                      /* +0x02 */
    s16 r;                      /* +0x04 */
    u8 unk06[0x28 - 0x06];
    s32 unk28;                  /* +0x28 */
} Ovl203ActDef;

typedef struct {
    s16 v[5];
} S16x5;

typedef struct {
    u8 used;                    /* +0x00 */
    u8 slot;                    /* +0x01 actor index */
    u16 ap;                     /* +0x02 */
    s16 x;                      /* +0x04 */
    s16 y;                      /* +0x06 */
    s16 z;                      /* +0x08 */
    s16 type;                   /* +0x0A */
    SVECTOR rot;                /* +0x0C */
    u16 flags;                  /* +0x14 */
    u8 unk16[4];
    u16 ap_max;                 /* +0x1A */
    u8 unk1c[6];
    S16x5 extra;                /* +0x22 */
} Ovl203Spawn;

typedef struct {
    Ovl203ActDef defs[4];       /* 0x8019B4A0 */
    AcEntity ents[2];           /* 0x8019B550 */
} Ovl203Actors;

extern Ovl203Actors D_8019B4A0;
extern SVECTOR D_80039C78;

#define F(T, off) (*(T *)((u8 *)e + (off)))

/* Set up actor e from spawn record rec. */
void func_80077894(AcEntity *e, Ovl203Spawn *rec) {
    Ovl203ActDef *def;

    def = &D_8019B4A0.defs[rec->type];
    rec->used = 1;
    rec->slot = e - D_8019B4A0.ents;
    e->def = rec;
    e->geom = def;
    F(u8, 0x3A) = 3;
    F(s16, 0x38) = 0;
    F(u16, 0x162) = F(u16, 0x164) = rec->ap_max;
    F(u16, 0x160) = rec->ap;
    F(s16, 0x28) = F(s16, 0x6A) = def->w;
    F(s16, 0x2A) = F(s16, 0x6C) = def->h;
    F(s16, 0x1E) = def->r;
    F(s32, 0x80) = def->unk28;
    F(u16, 0x32) = (rec->flags >> 4) & 2;
    e->pos.x = rec->x;
    e->pos.y = rec->y - 1;
    e->pos.z = rec->z;
    e->pos.w = rec->type;
    F(S16x5, 0xFC) = rec->extra;
    F(SVECTOR, 0x10) = rec->rot;
    F(s16, 0x2E) = 32767;
    F(s16, 0x2C) = e->pos.y;
    F(s16, 0x30) = F(s16, 0x16);
    F(s32, 0x48) = 0;
    F(u8, 0x64) = 0;
    F(s16, 0x6E) = 0;
    F(u8, 0x65) = 255;
    F(u8, 0x68) = 4;
    F(SVECTOR, 0x76) = D_80039C78;
    F(s16, 0x7C) = -1;
    F(s16, 0x72) = 0;
    F(s32, 0x60) = 0;
    F(u16, 0x148) = 0;
    F(s32, 0x15C) = 0;
    F(u8, 0x3E) = 0;
    F(u8, 0x3F) = 0;
    F(u8, 0x40) = 255;
    F(u8, 0x3D) = 0;
    F(u8, 0x3C) = 0;
    F(u8, 0x41) = 0;
    F(s32, 0x4C) = 0;
    F(s32, 0x50) = 0;
    F(s32, 0x54) = 0;
    F(s32, 0x58) = 0;
    F(s32, 0x5C) = 0;
}
