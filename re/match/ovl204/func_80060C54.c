#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_SetRotMatrix(r0) __asm__ volatile ("lw $12, 0(%0)\n\tlw $13, 4(%0)\n\tctc2 $12, $0\n\tctc2 $13, $1\n\tlw $12, 8(%0)\n\tlw $13, 12(%0)\n\tlw $14, 16(%0)\n\tctc2 $12, $2\n\tctc2 $13, $3\n\tctc2 $14, $4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ("lw $12, 20(%0)\n\tlw $13, 24(%0)\n\tctc2 $12, $5\n\tlw $14, 28(%0)\n\tctc2 $13, $6\n\tctc2 $14, $7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("lw $9, %0\n\tnop\n\tmtc2 $9, $8" : : "m"(r0) : "$9")
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_rt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A486012")
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 0(%1)\n\tswc2 $14, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {
    u8 rgb[4];                  /* +0x00 */
    u32 uv0;                    /* +0x04 u0 v0 clut */
    u32 uv1;                    /* +0x08 u1 v1 tpage */
    u16 uv2;                    /* +0x0C */
    u16 uv3;                    /* +0x0E */
    SVECTOR v[4];               /* +0x10 quad corners */
} BbFrame;                      /* 0x30 */

typedef struct {
    s16 x, y, z;                /* +0x00 */
    s16 type;                   /* +0x06 -1 = free */
    s16 ws;                     /* +0x08 world structure (visibility, light) */
    u8 base;                    /* +0x0A first frame */
    u8 n;                       /* +0x0B frames */
    u8 rate;                    /* +0x0C frame period (0 = still) */
    u8 cur;                     /* +0x0D current frame */
    u8 pad[2];
} BbObj;                        /* 0x10 */

typedef struct {
    u8 lm[0x50];
    s16 col[9];
    u8 r, g, b;                 /* +0x62 back colour */
    u8 pad65;
    u16 fog;                    /* +0x66 */
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
    u8 unke04[0xE64 - 0xE04];
    VECTOR camt;                /* +0xE64 (0x801B4FBC) */
    u8 unke74[0xFF4 - 0xE74];
    BbObj obj[256];             /* +0xFF4 (0x801B514C) */
} BbWorld;

typedef struct {
    u8 *cur;                    /* +0x00 */
    u8 *end;                    /* +0x04 */
    u32 *ot;                    /* +0x08 */
    SVECTOR d;                  /* +0x0C */
    SVECTOR lv;                 /* +0x14 light vector */
    MATRIX m1;                  /* +0x1C */
    MATRIX m2;                  /* +0x3C */
    s32 fog;                    /* +0x5C */
    s32 fc;                     /* +0x60 */
    s32 count;                  /* +0x64 */
    SVECTOR cp;                 /* +0x68 camera position */
} BbScratch;

typedef struct {
    s32 w[5];
} RotPart;

typedef struct {
    PrimBuf *buf;               /* +0x0000 */
    u32 *ot;                    /* +0x0004 */
    u8 unk8[0x7420 - 8];
    RotPart rot;                /* +0x7420 (0x801B50EC) */
} DrawBlk;

extern DrawBlk D_801ADCCC;
extern RotPart D_801B4F9C;

extern BbWorld D_801B4158;
extern WStruct D_801D4488[];
extern BbFrame *D_800896FC[];
extern u32 D_801B4114;
extern s32 func_80060B08(s32 a, BbObj *e, s32 w, s32 h);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);

/* draw the animated billboards of the 256-entry object table */
void func_80060C54(s32 arg)
{
    BbScratch *scr = (BbScratch *)0x1F800000;
    BbObj *e;
    BbFrame *fr;
    POLY_FT4 *p;
    LightSet *lt;
    s32 *src;
    s32 *dst;
    s32 i;
    s32 otz;
    u32 f;
    s32 last;

    last = -1;
    scr->ot = D_801ADCCC.ot;
    scr->cur = D_801ADCCC.buf->cur;
    scr->end = D_801ADCCC.buf->end;
    {
        s32 *s1;
        s32 *d1;
        s32 n1;

        s1 = D_801ADCCC.rot.w;
        d1 = (s32 *)&scr->m1;
        for (n1 = 5; n1 != 0; n1--) {
            *d1++ = *s1++;
        }
    }
    {
        s32 *s2;
        s32 *d2;
        s32 n2;

        s2 = D_801B4F9C.w;
        d2 = (s32 *)&scr->m2;
        for (n2 = 5; n2 != 0; n2--) {
            *d2++ = *s2++;
        }
    }
    scr->lv.vx = 4096;
    scr->lv.vz = 0;
    scr->lv.vy = 0;
    scr->count = 256;
    scr->m2.t[0] = D_801B4158.camt.vx;
    scr->m2.t[1] = D_801B4158.camt.vy;
    scr->m2.t[2] = D_801B4158.camt.vz;
    e = D_801B4158.obj;
    scr->cp.vx = ((MATRIX *)&D_801B4F9C)->t[0];
    scr->cp.vy = ((MATRIX *)&D_801B4F9C)->t[1];
    scr->cp.vz = ((MATRIX *)&D_801B4F9C)->t[2];
    do {
        if (e->type == -1) {
            continue;
        }
        i = e->ws;
        if (!(D_801B4158.vis[i >> 5] & (1 << (i & 0x1F)))) {
            continue;
        }
        gte_SetRotMatrix(&scr->m2);
        gte_SetTransMatrix(&scr->m2);
        if (func_80060B08(arg, e, 500, 500) == 0) {
            continue;
        }
        if (D_801D4488[e->ws].light != last) {
            last = D_801D4488[e->ws].light;
            lt = &D_801B4158.light[last];
            SetBackColor(lt->r, lt->g, lt->b);
            f = lt->fog;
            scr->fog = f;
            if (f != 0) {
                f *= 2;
                SetFogNearFar(f >> 4, f >> 2, 80);
            }
        }
        scr->d.vx = e->x - scr->cp.vx;
        scr->d.vy = e->y - scr->cp.vy;
        scr->d.vz = e->z - scr->cp.vz;
        gte_ldv0(&scr->d);
        gte_rt();
        gte_stlvnl(scr->m1.t);
        gte_SetRotMatrix(&scr->m1);
        gte_SetTransMatrix(&scr->m1);
        p = (POLY_FT4 *)scr->cur;
        scr->cur = (u8 *)(p + 1);
        fr = D_800896FC[e->type] + e->base + e->cur;
        gte_ldv3(&fr->v[0], &fr->v[1], &fr->v[2]);
        gte_rtpt();
        *(u32 *)&p->u0 = fr->uv0;
        *(u32 *)&p->u1 = fr->uv1;
        gte_stsxy3(&p->x0, &p->x1, &p->x2);
        gte_stszotz(&otz);
        if (otz != 0) {
            if (scr->fog == 0) {
                scr->fc = 0;
            } else {
                gte_stdp(&scr->fc);
            }
            gte_ldv0(&fr->v[3]);
            gte_rtps();
            *(u16 *)&p->u2 = fr->uv2;
            *(u16 *)&p->u3 = fr->uv3;
            setlen(p, 9);
            gte_stsxy(&p->x3);
            gte_ldv0(&scr->lv);
            gte_ldrgb(fr->rgb);
            gte_lddp(scr->fc);
            gte_ncds();
            gte_strgb(&p->r0);
            otz >>= 1;
            if (scr->d.vy <= 0) {
                otz += 120;
            } else {
                otz += 100;
            }
            addPrim(scr->ot + otz, p);
            if (e->rate != 0 && (D_801B4114 % e->rate) == 0) {
                if (++e->cur >= e->n) {
                    e->cur = 0;
                }
            }
        } else {
            scr->cur -= sizeof(POLY_FT4);
        }
    } while (e++, --scr->count != 0);
    D_801ADCCC.buf->cur = scr->cur;
}
