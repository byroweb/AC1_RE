#include "ac1/ac1.h"
#include "ac1/gte.h"
#include "ac1/prim2d.h"

#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12, 0(%0)\n\t" \
    "lw $13, 4(%0)\n\t" \
    "ctc2 $12, $0\n\t" \
    "ctc2 $13, $1\n\t" \
    "lw $12, 8(%0)\n\t" \
    "lw $13, 12(%0)\n\t" \
    "lw $14, 16(%0)\n\t" \
    "ctc2 $12, $2\n\t" \
    "ctc2 $13, $3\n\t" \
    "ctc2 $14, $4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12, 20(%0)\n\t" \
    "lw $13, 24(%0)\n\t" \
    "ctc2 $12, $5\n\t" \
    "lw $14, 28(%0)\n\t" \
    "ctc2 $13, $6\n\t" \
    "ctc2 $14, $7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_rtv0tr() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A486012")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25, 0(%0)\n\t" \
    "swc2 $26, 4(%0)\n\t" \
    "swc2 $27, 8(%0)" : : "r"(r0) : "memory")
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 0(%1)\n\tswc2 $14, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_ldir0(r0) __asm__ volatile ("lw $9, %0\n\tmtc2 $9, $8" : : "m"(*(r0)) : "$9")
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x30 */
    u32 rgbc;                   /* +0x00 */
    u32 uv0;                    /* +0x04 uv0 + clut */
    u32 uv1;                    /* +0x08 uv1 + tpage */
    u16 uv2;                    /* +0x0C */
    u16 uv3;                    /* +0x0E */
    SVECTOR v[4];               /* +0x10 */
} SprFrame;

typedef struct {                /* 0x10 */
    s16 pos[3];                 /* +0x00 */
    s16 kind;                   /* +0x06 -1 = free */
    u16 ws;                     /* +0x08 world struct */
    u8 set;                     /* +0x0A */
    u8 frames;                  /* +0x0B */
    u8 rate;                    /* +0x0C */
    u8 frame;                   /* +0x0D */
    u8 pad0e[2];
} SprObj;

typedef struct {
    u8 lm[0x50];
    s16 col[9];                 /* +0x50 */
    u8 r, g, b;                 /* +0x62 */
    u8 pad65;
    u16 fog;                    /* +0x66 */
} LightSet;                     /* 0x68 */

typedef struct {
    u8 pad[0x30];
    u8 light;                   /* +0x30 */
    u8 pad31[0x0F];
} WStruct;                      /* 0x40 */

typedef struct {                /* scratchpad 0x1F800000 */
    u8 *cur;                    /* +0x00 */
    u8 *end;                    /* +0x04 */
    u32 *ot;                    /* +0x08 */
    SVECTOR d;                  /* +0x0C */
    SVECTOR n;                  /* +0x14 */
    MATRIX m1;                  /* +0x1C */
    MATRIX m2;                  /* +0x3C */
    u32 fog;                    /* +0x5C */
    s32 ir0;                    /* +0x60 */
    s32 count;                  /* +0x64 */
    s16 eye[3];                 /* +0x68 */
} SprScr;

typedef struct {
    u8 pad00[4];
    u8 *end;                    /* +0x04 */
    u8 *cur;                    /* +0x08 */
} PrimBuf2;

typedef struct {
    PrimBuf2 *buf;              /* 0x8019D95C */
    u32 *ot;                    /* 0x8019D960 */
    u8 pad08[0x6484];
    u32 vis[0x40];              /* 0x801A3DE8 world-struct visibility bits */
    u8 pad1[4];
    LightSet light[0x20];       /* 0x801A3EEC */
    u8 pad2[0x18];
    MATRIX cam;                 /* 0x801A4C04 */
    VECTOR eye;                 /* 0x801A4C24 */
    u8 pad3[0x120];
    u32 bill[5];                /* 0x801A4D54 */
    u8 pad4[0x4C];
    SprObj obj[256];            /* 0x801A4DB4 */
} RenderCtx;

extern RenderCtx D_8019D95C;
extern MATRIX D_801A4C04;
extern WStruct D_801C40F0[];
extern SprFrame *D_80089980[];
extern u32 D_801A3DA4;
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);
extern s32 func_8006137C(s32 a, s16 *pos, s32 w, s32 h);

void func_800614C8(s32 arg)
{
    SprScr *s;
    s32 z;
    u32 *src;
    u32 *dst;
    s32 i;
    SprObj *e;
    s32 last;
    LightSet *lt;
    s32 li;
    u8 *p;
    SprFrame *f;
    VECTOR *ep;
    s32 w;
    u32 fog;

    last = -1;
    s = (SprScr *)0x1F800000;
    s->ot = D_8019D95C.ot;
    s->cur = D_8019D95C.buf->cur;
    s->end = D_8019D95C.buf->end;
    src = D_8019D95C.bill;
    dst = (u32 *)&s->m1;
    i = 5;
    do {
        *dst++ = *src++;
    } while (--i != 0);
    src = (u32 *)&D_801A4C04;
    dst = (u32 *)&s->m2;
    i = 5;
    do {
        *dst++ = *src++;
    } while (--i != 0);
    ep = &D_8019D95C.eye;
    e = D_8019D95C.obj;
    s->n.vx = 4096;
    s->n.vz = 0;
    s->n.vy = 0;
    s->count = 256;
    s->m2.t[0] = ep->vx;
    s->m2.t[1] = ep->vy;
    s->m2.t[2] = ep->vz;
    s->eye[0] = D_8019D95C.cam.t[0];
    s->eye[1] = D_8019D95C.cam.t[1];
    s->eye[2] = D_8019D95C.cam.t[2];
    do {
        if (e->kind != -1 && (w = (s16)e->ws, D_8019D95C.vis[w >> 5] & (1 << (w & 0x1F)))) {
            gte_SetRotMatrix(&s->m2);
            gte_SetTransMatrix(&s->m2);
            if (func_8006137C(arg, e->pos, 500, 500) != 0) {
                li = D_801C40F0[(s16)e->ws].light;
                if (li != last) {
                    last = li;
                    lt = &D_8019D95C.light[last];
                    SetBackColor(lt->r, lt->g, lt->b);
                    s->fog = fog = lt->fog;
                    if (fog != 0) {
                        fog *= 2;
                        SetFogNearFar(fog >> 4, fog >> 2, 160);
                    }
                }
                s->d.vx = e->pos[0] - s->eye[0];
                s->d.vy = e->pos[1] - s->eye[1];
                s->d.vz = e->pos[2] - s->eye[2];
                gte_ldv0(&s->d);
                gte_rtv0tr();
                gte_stlvnl(s->m1.t);
                gte_SetRotMatrix(&s->m1);
                gte_SetTransMatrix(&s->m1);
                p = s->cur;
                s->cur = p + 40;
                f = (SprFrame *)((u8 *)D_80089980[e->kind] + e->set * 48 + e->frame * 48);
                gte_ldv3(&f->v[0], &f->v[1], &f->v[2]);
                gte_rtpt();
                *(u32 *)(p + 12) = f->uv0;
                *(u32 *)(p + 20) = f->uv1;
                gte_stsxy3(p + 8, p + 16, p + 24);
                gte_stszotz(&z);
                if (z != 0) {
                    if (s->fog == 0) {
                        s->ir0 = 0;
                    } else {
                        gte_stdp(&s->ir0);
                    }
                    gte_ldv0(&f->v[3]);
                    gte_rtps();
                    *(u16 *)(p + 28) = f->uv2;
                    *(u16 *)(p + 36) = f->uv3;
                    p[3] = 9;
                    gte_stsxy(p + 32);
                    gte_ldv0(&s->n);
                    gte_ldrgb(f);
                    gte_ldir0(&s->ir0);
                    gte_ncds();
                    gte_strgb(p + 4);
                    z >>= 1;
                    if (s->d.vy <= 0) {
                        z += 120;
                    } else {
                        z += 100;
                    }
                    addPrim(s->ot + z, p);
                    if (e->rate != 0 && D_801A3DA4 % e->rate == 0) {
                        if (++e->frame >= e->frames) {
                            e->frame = 0;
                        }
                    }
                } else {
                    s->cur -= 40;
                }
            }
        }
        e++;
    } while (--s->count != 0);
    D_8019D95C.buf->cur = s->cur;
}
