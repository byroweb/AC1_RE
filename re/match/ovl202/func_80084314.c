#include "ac1/ac1.h"

/* homing-missile view of a projectile slot (kind-specific layout) */
typedef struct {
    s16 x, y, z;                /* +0x00 */
    s16 free;                   /* +0x06 */
    s32 owner;                  /* +0x08 */
    s16 unk0c;                  /* +0x0C */
    s16 section;                /* +0x0E */
    u8 kind;                    /* +0x10 */
    u8 unk11;
    s16 unk12;                  /* +0x12 */
    void *think;                /* +0x14 */
    u8 unk18[6];                /* +0x18 */
    SVECTOR vel;                /* +0x1E (vy doubles as the thrust) */
    SVECTOR tgt;                /* +0x26 last seen target position */
    u8 unk2e[6];
    s32 target;                 /* +0x34 */
    u16 timer;                  /* +0x38 */
    u16 t_fire;                 /* +0x3A */
    u16 type;                   /* +0x3C */
    u16 flags;                  /* +0x3E */
    u8 unk40[0x78];
    s16 ang[2];                 /* +0xB8 */
    s16 unkbc;
    s16 unkbe;                  /* +0xBE */
    u8 unkc0[6];
    u16 unkc6;                  /* +0xC6 */
} Missile;

typedef struct {
    u16 life;                   /* +0x00 */
    u16 unk02;
    u16 unk04;
    u16 unk06;
    u16 unk08;                  /* +0x08 */
    u16 unk0a;                  /* +0x0A */
    u16 guided;                 /* +0x0C */
    u16 maxspd;                 /* +0x0E */
    s16 turn;                   /* +0x10 */
    s16 cone;                   /* +0x12 */
    u16 unk14;                  /* +0x14 */
    u16 unk16;                  /* +0x16 */
    u8 unk18[8];
    u32 fx;                     /* +0x20 */
    u32 trail;                  /* +0x24 */
} MissileDef;

typedef struct {
    s16 a;
    u16 b;
    u8 c;
    u8 d;
} Tint6;

extern MissileDef D_8009019C[];
extern s32 D_801A5DC4;
extern u8 D_801E8B74[];

extern s16 func_800132CC(s32 cur, s32 want, s32 rate);
extern void func_80014774(SVECTOR *rot, SVECTOR *dir);
extern s32 func_800148E4(s32 a, s32 b, s32 range);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern void func_80015258(void *p, s32 a, s32 spd, SVECTOR *tgt, SVECTOR *d, SVECTOR *out, s32 b, s32 c);
extern void func_8004C98C(s32 a, s32 id, void *p, SVECTOR *v, s32 b, s32 c, s32 d, s32 e, s32 f);
extern void func_8004CE44(s32 sfx, void *pos, s32 vol, s32 arg3);
extern s32 func_800742DC(s32 target, SVECTOR *out);
extern void func_800835E8(s32 owner, s32 **hit, void *a, SVECTOR *v, SVECTOR *pos);
extern s32 **func_80083FA0(void *p, s32 m, s32 maxspd, SVECTOR *vel, SVECTOR *acc, SVECTOR *out);
extern void func_800841F8(void *p, MissileDef *def, SVECTOR *pos, void *at, SVECTOR *dir);
extern Projectile *func_8008589C(s32 owner, void *pos, s32 a, s32 b, s32 c, s32 section);
extern Projectile *func_800862E8(s32 owner, void *a, void *p, SVECTOR *rot, SVECTOR *vel, s32 sp, s32 b0, s32 b1, s32 tgt);
extern void func_80086854(s32 owner, void *pos, u32 *fx);
extern void func_80086DDC(s32 a, SVECTOR *pos, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h, s32 i);
extern void func_800873F8(s32 **hit, SVECTOR *pos, s32 a, s32 b, s32 c, s32 d);
extern void func_80087798(s32 a, SVECTOR *pos, void *t, s32 b, s32 c);
extern void func_80089F7C(s32 target, s32 a, s32 owner);

void func_80084314(Missile *p)
{
    SVECTOR pos;
    SVECTOR tgt;
    SVECTOR ang;
    SVECTOR dir;
    SVECTOR out;
    SVECTOR vel;
    SVECTOR d;
    SVECTOR rot;
    SVECTOR aim;
    MissileDef *def;
    s32 guided;
    s32 i;
    s32 **hit;
    s32 *fc;

    def = &D_8009019C[p->type];
    if (p->timer == 10) {
        p->owner &= 0x7FFFFFFF;
    }
    if (p->timer >= def->life) {
        p->ang[0] = func_800132CC(p->ang[0], 0x400, 0x10);
        goto thrust;
    }
    if (p->timer < p->t_fire) {
    thrust:
        p->vel.vy += 8;
        guided = 0;
        if (def->maxspd < p->vel.vy) {
            p->vel.vy = def->maxspd;
        }
        goto fly;
    }
    if (p->timer == p->t_fire) {
        if (p->unkbe != 0) {
            func_8004CE44(2, p, 0x6E, 0);
        }
    } else {
        func_8004C98C(0xC, ((Projectile *)p - g_proj_pool) + 0x2000, p, &p->vel, 0x3C, 4000, 4300, 0, 500);
    }
    if (p->timer == p->t_fire + 6 && p->type == 3) {
        i = 3;
        do {
            rot = *(SVECTOR *)p->ang;
            if (i < 2) {
                rot.vy = (rot.vy + 448) % 4096;
            } else {
                rot.vy = (rot.vy - 448) % 4096;
            }
            if (i & 1) {
                rot.vx = (rot.vx - 512) % 4096;
            }
            vel = p->vel;
            func_800862E8(p->owner, p->unk18, p, &rot, &vel, 0x30, 0, 1, p->target)->unkbe = 0;
        } while (--i != -1);
        p->unk12 = 0;
        return;
    }
    guided = def->guided;
    if (func_800742DC(p->target, &tgt) != 0 && !(p->flags & 2)) {
        d.vx = tgt.vx - p->tgt.vx;
        d.vy = tgt.vy - p->tgt.vy;
        d.vz = tgt.vz - p->tgt.vz;
        p->tgt = tgt;
        func_80015258(p, 0, def->maxspd, &tgt, &d, &aim, 0, def->unk14);
        aim.vx -= p->x;
        aim.vy -= p->y;
        aim.vz -= p->z;
        func_80014AE4(aim.vx, aim.vy, aim.vz, &ang);
        if (func_800148E4(p->ang[0], ang.vx, def->cone) != 0 && func_800148E4(p->ang[1], ang.vy, def->cone) != 0) {
            p->flags |= 1;
        } else if (p->flags & 1) {
            p->flags |= 2;
        }
    } else {
        ang.vx = p->ang[0];
        ang.vy = p->ang[1];
        if (rand() < 3277) {
            ang.vx = (s16)(ang.vx - 32) + (rand() & 0x3F);
            ang.vy = (s16)(ang.vy - 32) + (rand() & 0x3F);
        }
    }
    p->ang[0] = func_800132CC(p->ang[0], ang.vx, def->turn);
    p->ang[1] = func_800132CC(p->ang[1], ang.vy, def->turn);
    ang.vx = -p->ang[0];
    ang.vy = p->ang[1];
    func_80014774(&ang, &dir);
fly:
    pos = *(SVECTOR *)p;
    hit = func_80083FA0(p, guided, def->maxspd, &p->vel, &dir, &out);
    if (hit != 0) {
        func_800873F8(hit, &out, def->trail & 0x3FF, (def->trail >> 10) & 0xFF, (def->trail >> 18) & 0x1F, 1);
        func_800835E8(p->owner, hit, p->unk18, &p->vel, &out);
        if ((u8 *)hit >= D_801E8B74 && (u8 *)hit <= D_801E8B74 + 7140 && (*(u16 *)((u8 *)*hit + 0xE) & 1)) {
            s32 n;
            s32 q;

            n = (def->fx >> 17) & 0x3FF;
            q = n >> 2;
            func_80086DDC(0, &out, q, def->unk16, 40, q, q * 4, n >> 3, q * 2, 64);
        }
        func_80086854(0, &out, &def->fx);
        if (p->type >= 2 && p->type < 8) {
            *(Tint6 *)&rot = *(Tint6 *)p->unk18;
            ((Tint6 *)&rot)->b >>= 3;
            ((Tint6 *)&rot)->c >>= 2;
            func_80087798(0, &out, &rot, 5, 1);
        }
        if (guided != 0) {
            func_800841F8(p, def, &pos, &out, &dir);
        }
        p->unk12 = 0;
    } else {
        if (p->unk12 == 0) {
            func_80086854(0, p, &def->fx);
            p->unk12 = 0;
        }
        if (guided != 0) {
            p->unkc6 = (p->unkc6 == 0);
            func_8008589C(0, p, def->unk08, def->unk0a, 0, p->section)->unk12 = 0;
            fc = &D_801A5DC4;
            if (*fc & 1) {
                func_800841F8(p, def, &pos, p, &dir);
            }
            if (!(*fc & 7)) {
                func_80089F7C(p->target, 1, p->owner | 0x80000000);
            }
        } else {
            p->unkc6 = 0;
        }
    }
    p->timer++;
}
