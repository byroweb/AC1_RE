#include "ac1/ac1.h"
/* ovl202 0x80054750 - set up / advance the skeletal animation state of model `id`:
 * allocate the state and its part array on first use, apply and drop the pose
 * (ang->pad), switch the secondary animation, start a blend when the main animation
 * changes, run the command streams up to the current frames (func_80053A54 /
 * func_80053FAC), blend the parts toward their targets with the GTE (GPF), smooth the
 * rotations and rebuild the part matrices. */

#ifndef NULL
#define NULL ((void *)0)
#endif
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ldsv(r0) __asm__ volatile ( \
    "lhu $12, 0(%0)\n\t" \
    "lhu $13, 2(%0)\n\t" \
    "lhu $14, 4(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_gpf12() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B98003D")
#define gte_stsv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sh $12, 0(%0)\n\t" \
    "sh $13, 2(%0)\n\t" \
    "sh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct Part7C {
    s16 base[9];                /* +0x00 trans, rot, scale at rest */
    s16 parent_idx;             /* +0x12 -1: unused */
    s16 m[3][3];                /* +0x14 */
    u16 flags;                  /* +0x26 */
    u8 pad28[0x20];
    struct Part7C *parent;      /* +0x48 */
    s16 trans[3];               /* +0x4C */
    s16 rot[3];                 /* +0x52 */
    s16 scale[3];               /* +0x58 */
    s16 rot2[3];                /* +0x5E blend / smoothing target */
    s16 trans2[3];              /* +0x64 */
    u8 pad6a[0x12];
} Part7C;

typedef struct {
    u16 part;
    u16 base[3];
    s16 neg[3];
    s16 pos[3];
} PoseEnt;

typedef struct {
    u16 n;
    s16 a[3];
    s16 lo[3];
    s16 hi[3];
    PoseEnt e[1];
} Pose;

typedef struct {
    u8 pad00[5];
    u8 nparts;                  /* +0x05 */
    u8 pad06[6];
    s32 skel;                   /* +0x0C */
    s32 anims;                  /* +0x10 */
    s32 poses;                  /* +0x14 */
    s32 anims2;                 /* +0x18 */
} AnimModel;

typedef struct AnimState {
    s16 state;                  /* +0x00 2: up to date */
    u16 id;                     /* +0x02 */
    u16 anim;                   /* +0x04 */
    u16 frame;                  /* +0x06 */
    s16 frac;                   /* +0x08 */
    s16 blend;                  /* +0x0A */
    s16 step;                   /* +0x0C */
    s16 pose;                   /* +0x0E */
    u16 anim2;                  /* +0x10 */
    u16 frame2;                 /* +0x12 */
    u16 *strm;                  /* +0x14 */
    u16 *strm2;                 /* +0x18 */
    AnimModel *model;           /* +0x1C */
    Part7C *parts;              /* +0x20 */
    struct AnimState **owner;   /* +0x24 */
} AnimState;

extern AnimModel *D_801A5F34[];
extern u8 D_800A1228[];
extern Part7C *func_8001606C(void *heap, s32 size, Part7C **owner, s32 flags);
extern void func_80015538(void *p, s32 v, s32 n);
extern void func_800144FC(void *src, void *out);
extern MATRIX *RotMatrix(SVECTOR *r, MATRIX *m);
extern void func_80053950(void *p, Part7C *parts);
extern u16 *func_80053A54(u16 *p, Part7C *parts, s32 from, s32 to, s32 mask);
extern void func_80053FAC(u16 *p, Part7C *parts, s32 w, s32 mask);
extern void func_8005460C(Part7C *p, u32 n);
extern void func_80054660(s16 *p, s32 x, s32 *idx, s32 *frac);
extern s32 func_800546E0(s32 base, s32 a, s32 b, s32 d);
extern void func_800558A8(AnimState *st);
extern void func_800558F4(void);
extern AnimState *func_800559D0(void);

#define POSE(m, i) ((Pose *)((u8 *)(m) + ((s32 *)((u8 *)(m) + (m)->poses))[i]))
#define ANIM2(m, i) ((u16 *)((u8 *)(m) + ((s32 *)((u8 *)(m) + (m)->anims2))[i]))

AnimState *func_80054750(AnimState **pp, s32 id, s32 anim, s32 time, s32 blend,
                         s32 anim2, s32 time2, SVECTOR *ang)
{
    SVECTOR d;
    s32 frame;
    s32 frac;
    s32 frame2;
    s32 frac2;
    AnimState *st;
    AnimModel *model;
    Part7C *q;
    Pose *pose;
    Pose *pose1;
    Pose *pose2;
    PoseEnt *ent;
    u16 *g;
    u16 *g1;
    u16 *g2;
    s16 *a;
    s32 *b;
    s32 n;
    s32 n1;
    Part7C *q10;
    s32 n10;
    Part7C *q2;
    Part7C *q8;
    Part7C *q4;
    Part7C *q3;
    s32 n3;
    s32 n4;
    Part7C *q5;
    s32 n5;
    Part7C *q6;
    s32 n6;
    Part7C *q7;
    s32 n7;
    Part7C *q9;
    Part7C *q11;
    s32 n11;
    s32 n9;
    s32 lo;
    s32 hi;
    s32 dd0;
    s32 dd1;
    s32 dd2;
    s32 k;
    s32 bl;
    s32 x;
    Part7C *t;
    Part7C *t2;
    s32 reset;
    s32 reset2;
    SVECTOR *sv;

    reset2 = 0;
    st = *pp;
    reset = 0;
    if (st != NULL) {
        if (st->state == 2) {
            return st;
        }
    }
    model = D_801A5F34[id];
    if (st == NULL) {
        st = func_800559D0();
        if (st == NULL) {
            return NULL;
        }
    init:
        st->id = id;
        st->owner = pp;
    again:
        t = func_8001606C(D_800A1228, model->nparts * 0x7C, &st->parts, 0);
        st->parts = t;
        if (t == NULL) {
            func_800558F4();
            goto again;
        }
        q = t;
        func_80015538(q, 0, model->nparts * 31);
        n1 = model->nparts;
        while (--n1 != -1) {
            q->flags = 0;
            q++;
        }
        func_80053950((u8 *)model + model->skel, st->parts);
        *pp = st;
        if (ang->pad != -1) {
            pose = POSE(model, ang->pad);
            d = *ang;
            n = pose->n;
            ent = pose->e;
            d.vx = func_800546E0(pose->a[0], pose->lo[0], pose->hi[0], ang->vx);
            d.vy = func_800546E0(pose->a[1], pose->lo[1], pose->hi[1], ang->vy);
            d.vz = func_800546E0(pose->a[2], pose->lo[2], pose->hi[2], ang->vz);
            while (--n != -1) {
                t2 = &st->parts[ent->part];
                if (!(t2->flags & 0x40)) {
                    q2 = &st->parts[ent->part];
                    x = d.vx;
                    k = x < 0 ? ent->neg[0] : ent->pos[0];
                    q2->rot2[0] = q2->rot[0] = ent->base[0] + ((x * k) >> 12);
                    x = d.vy;
                    k = x < 0 ? ent->neg[1] : ent->pos[1];
                    q2->rot2[1] = q2->rot[1] = ent->base[1] + ((x * k) >> 12);
                    x = d.vz;
                    k = x < 0 ? ent->neg[2] : ent->pos[2];
                    q2->rot2[2] = q2->rot[2] = ent->base[2] + ((x * k) >> 12);
                }
                ent++;
            }
        }
        st->pose = ang->pad;
        st->model = NULL;
    } else if (st->id != id) {
        func_800558A8(st);
        st->anim = 0xFF;
        reset = 1;
        goto init;
    } else {
        if (st->pose != ang->pad) {
            if (st->pose != -1) {
                pose1 = POSE(model, st->pose);
                q = &st->parts[pose1->e[0].part];
                n = pose1->n;
                ent = pose1->e;
                if (q->parent != NULL) {
                    func_800144FC((u8 *)q->parent + 0x28, &d);
                    q->rot2[0] = q->rot[0] = q->rot[0] - d.vx;
                    q->rot2[1] = q->rot[1] = q->rot[1] - d.vy;
                    q->rot2[2] = q->rot[2] = q->rot[2] - d.vz;
                } else {
                    q->rot2[0] = q->rot[0];
                    q->rot2[1] = q->rot[1];
                    q->rot2[2] = q->rot[2];
                }
                while (--n != -1) {
                    q = &st->parts[ent->part];
                    ent++;
                    q->flags = (q->flags & 0xFFC7) | 4;
                }
                st->pose = -1;
                reset = 1;
            }
            if (ang->pad != -1) {
                pose2 = POSE(model, ang->pad);
                ent = pose2->e;
                n = pose2->n;
                q = &st->parts[ent->part];
                func_800144FC((u8 *)q + 0x28, q->rot);
                for (;;) {
                    n--;
                    q->rot2[0] = q->rot[0];
                    q->rot2[1] = q->rot[1];
                    q->rot2[2] = q->rot[2];
                    if (n == 0) {
                        break;
                    }
                    ent++;
                    q = &st->parts[ent->part];
                }
            }
            st->pose = ang->pad;
        }
        if (st->anim2 != anim2) {
            if (st->anim2 != 0xFF) {
                g1 = ANIM2(model, st->anim2);
                n4 = *g1++;
                while (--n4 != -1) {
                    q4 = &st->parts[*g1++];
                    q4->flags = (q4->flags & 0xFFB7) | 4;
                }
                reset = 1;
            }
            if (anim2 != 0xFF) {
                g2 = ANIM2(model, anim2);
                n3 = *g2++;
                while (--n3 != -1) {
                    q3 = &st->parts[*g2++];
                    q3->flags = (q3->flags & 0xFFFB) | 0x48;
                    q3->base[3] = q3->rot2[0] = q3->rot[0];
                    q3->base[4] = q3->rot2[1] = q3->rot[1];
                    q3->base[5] = q3->rot2[2] = q3->rot[2];
                }
            } else {
                st->anim2 = 0xFF;
            }
            st->anim2 = anim2;
            reset2 = 1;
        }
        if (st->anim != anim) {
            st->step = 0;
            if (blend != 0) {
                q5 = st->parts;
                n5 = model->nparts;
                while (--n5 != -1) {
                    if (!(q5->flags & 0x50)) {
                        q5->flags = (q5->flags | 0x80) & 0xFFF3;
                        q5->rot2[0] = q5->rot[0];
                        q5->rot2[1] = q5->rot[1];
                        q5->rot2[2] = q5->rot[2];
                        q5->trans2[0] = q5->trans[0];
                        q5->trans2[1] = q5->trans[1];
                        q5->trans2[2] = q5->trans[2];
                    }
                    q5++;
                }
                st->step = blend;
                st->blend = 0x1000;
            }
        }
    }
    b = (s32 *)((u8 *)model + model->anims);
    a = (s16 *)((u8 *)b + b[anim]);
    func_80054660(a, time, &frame, &frac);
    a += a[1] + 2;
    if (reset || st->anim != anim || frame < st->frame || st->model != model) {
        q6 = st->parts;
        n6 = model->nparts;
        while (--n6 != -1) {
            q6->flags &= 0xFFFC;
            q6++;
        }
        st->strm = (u16 *)a;
        st->strm = func_80053A54(st->strm, st->parts, 0, frame, 8);
    } else if (st->frame < frame) {
        st->strm = func_80053A54(st->strm, st->parts, st->frame + 1, frame, 8);
    }
    st->frame = frame;
    st->anim = anim;
    st->frac = frac;
    if (st->anim2 != 0xFF) {
        g = ANIM2(model, st->anim2);
        g += *g + 1;
        func_80054660((s16 *)g, time2, &frame2, &frac2);
        g += g[1] + 2;
        if (reset2 || frame2 < st->frame2 || st->model != model) {
            st->strm2 = g;
            st->strm2 = func_80053A54(st->strm2, st->parts, 0, frame2, 0);
        } else if (st->frame2 < frame2) {
            st->strm2 = func_80053A54(st->strm2, st->parts, st->frame2 + 1, frame2, 0);
        }
        st->frame2 = frame2;
    }
    q7 = st->parts;
    n7 = model->nparts;
    while (--n7 != -1) {
        ((s32 *)q7->trans)[0] = ((s32 *)q7->base)[0];
        ((s32 *)q7->trans)[1] = ((s32 *)q7->base)[1];
        ((s32 *)q7->trans)[2] = ((s32 *)q7->base)[2];
        ((s32 *)q7->trans)[3] = ((s32 *)q7->base)[3];
        q7->scale[2] = q7->base[8];
        q7++;
    }
    func_80053FAC(st->strm, st->parts, frac, 8);
    if (st->pose != -1) {
        pose = POSE(model, st->pose);
        d = *ang;
        n = pose->n;
        ent = pose->e;
        if (st->parts[pose->e[0].part].parent != NULL) {
            SVECTOR o;

            func_800144FC((u8 *)st->parts[pose->e[0].part].parent + 0x28, &o);
            d.vx = func_800546E0(pose->a[0], pose->lo[0] + o.vx, pose->hi[0] + o.vx, ang->vx);
            d.vy = func_800546E0(pose->a[1], pose->lo[1] + o.vy, pose->hi[1] + o.vy, ang->vy);
            d.vz = func_800546E0(pose->a[2], pose->lo[2] + o.vz, pose->hi[2] + o.vz, ang->vz);
        } else {
            d.vx = func_800546E0(pose->a[0], pose->lo[0], pose->hi[0], ang->vx);
            d.vy = func_800546E0(pose->a[1], pose->lo[1], pose->hi[1], ang->vy);
            d.vz = func_800546E0(pose->a[2], pose->lo[2], pose->hi[2], ang->vz);
        }
        st->parts[ent->part].flags |= 0x20;
        while (--n != -1) {
            q8 = &st->parts[ent->part];
            if (!(q8->flags & 0x40)) {
                q8->flags |= 0x1C;
                x = d.vx;
                k = x < 0 ? ent->neg[0] : ent->pos[0];
                q8->rot[0] = ent->base[0] + ((x * k) >> 12);
                x = d.vy;
                k = x < 0 ? ent->neg[1] : ent->pos[1];
                q8->rot[1] = ent->base[1] + ((x * k) >> 12);
                x = d.vz;
                k = x < 0 ? ent->neg[2] : ent->pos[2];
                q8->rot[2] = ent->base[2] + ((x * k) >> 12);
            }
            ent++;
        }
    }
    if (st->anim2 != 0xFF) {
        func_80053FAC(st->strm2, st->parts, frac2, 0);
    }
    if (st->step != 0) {
        st->blend -= st->step;
        bl = st->blend;
        if (bl <= 0) {
            st->step = 0;
            q11 = st->parts;
            n11 = model->nparts;
            while (--n11 != -1) {
                if (q11->flags & 0x80) {
                    q11->flags = (q11->flags & 0xFF7F) | 1;
                }
                q11++;
            }
        } else {
            sv = (SVECTOR *)0x1F800000;
            q9 = st->parts;
            gte_lddp(bl);
            n9 = model->nparts;
            while (--n9 != -1) {
                if (q9->flags & 0x80) {
                    sv->vx = q9->trans2[0] - q9->trans[0];
                    sv->vy = q9->trans2[1] - q9->trans[1];
                    sv->vz = q9->trans2[2] - q9->trans[2];
                    if (sv->vx != 0 || sv->vy != 0 || sv->vz != 0) {
                        gte_ldsv(sv);
                        gte_gpf12();
                        gte_stsv(sv);
                        q9->trans[0] += sv->vx;
                        q9->trans[1] += sv->vy;
                        q9->trans[2] += sv->vz;
                    }
                    sv->vx = q9->rot2[0] - q9->rot[0];
                    sv->vy = q9->rot2[1] - q9->rot[1];
                    sv->vz = q9->rot2[2] - q9->rot[2];
                    if (sv->vx != 0 || sv->vy != 0 || sv->vz != 0) {
                        gte_ldsv(sv);
                        gte_gpf12();
                        gte_stsv(sv);
                        q9->rot[0] += sv->vx;
                        q9->rot[1] += sv->vy;
                        q9->rot[2] += sv->vz;
                        q9->flags |= 1;
                    }
                }
                q9++;
            }
        }
    }
    q10 = st->parts;
    n10 = model->nparts;
    while (--n10 != -1) {
        if (q10->parent_idx != -1) {
            if (q10->flags & 4) {
                q10->flags &= ~4;
                dd0 = q10->rot[0] - q10->rot2[0];
                if ((u32)(dd0 + 64) < 129) {
                    q10->rot2[0] = q10->rot[0];
                } else {
                    q10->rot2[0] = q10->rot2[0] + (dd0 >> 3);
                    q10->flags |= 4;
                }
                q10->rot[0] = q10->rot2[0];
                dd1 = q10->rot[1] - q10->rot2[1];
                if ((u32)(dd1 + 64) < 129) {
                    q10->rot2[1] = q10->rot[1];
                } else {
                    q10->rot2[1] = q10->rot2[1] + (dd1 >> 3);
                    q10->flags |= 4;
                }
                q10->rot[1] = q10->rot2[1];
                dd2 = q10->rot[2] - q10->rot2[2];
                if ((u32)(dd2 + 64) < 129) {
                    q10->rot2[2] = q10->rot[2];
                } else {
                    q10->rot2[2] = q10->rot2[2] + (dd2 >> 3);
                    q10->flags |= 4;
                }
                q10->rot[2] = q10->rot2[2];
                goto calc;
            }
            if (q10->flags & 1) {
            calc:
                RotMatrix((SVECTOR *)q10->rot, (MATRIX *)q10->m);
                if (q10->scale[0] != 4096) {
                    q10->m[0][0] = (q10->m[0][0] * q10->scale[0]) >> 12;
                    q10->m[1][0] = (q10->m[1][0] * q10->scale[0]) >> 12;
                    q10->m[2][0] = (q10->m[2][0] * q10->scale[0]) >> 12;
                }
                if (q10->scale[1] != 4096) {
                    q10->m[0][1] = (q10->m[0][1] * q10->scale[1]) >> 12;
                    q10->m[1][1] = (q10->m[1][1] * q10->scale[1]) >> 12;
                    q10->m[2][1] = (q10->m[2][1] * q10->scale[1]) >> 12;
                }
                if (q10->scale[2] != 4096) {
                    q10->m[0][2] = (q10->m[0][2] * q10->scale[2]) >> 12;
                    q10->m[1][2] = (q10->m[1][2] * q10->scale[2]) >> 12;
                    q10->m[2][2] = (q10->m[2][2] * q10->scale[2]) >> 12;
                }
                q10->flags &= ~1;
            }
            q10->flags |= 2;
        }
        q10++;
    }
    func_8005460C(st->parts, model->nparts);
    st->state = 2;
    st->model = model;
    return st;
}
