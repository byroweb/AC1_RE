#include "ac1/ac1.h"

#define gte_SetRotMatrix(r0) __asm__ volatile ("lw $12, 0(%0)\n\tlw $13, 4(%0)\n\tctc2 $12, $0\n\tctc2 $13, $1\n\tlw $12, 8(%0)\n\tlw $13, 12(%0)\n\tlw $14, 16(%0)\n\tctc2 $12, $2\n\tctc2 $13, $3\n\tctc2 $14, $4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ("lw $12, 20(%0)\n\tlw $13, 24(%0)\n\tctc2 $12, $5\n\tlw $14, 28(%0)\n\tctc2 $13, $6\n\tctc2 $14, $7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rtv0() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A486012")
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_ncds() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 0(%1)\n\tswc2 $14, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x30 flare sprite definition */
    u32 rgb;                    /* +0x00 colour + GPU code */
    u32 uv0;                    /* +0x04 u0 v0 clut */
    u32 uv1;                    /* +0x08 u1 v1 tpage */
    u16 uv2;                    /* +0x0C */
    u16 uv3;                    /* +0x0E */
    SVECTOR v[4];               /* +0x10 corners */
} FlareDef;

typedef struct {                /* 0x10 flare / lamp instance */
    s16 x, y, z;                /* +0x00 */
    s16 id;                     /* +0x06 def table, -1 = unused */
    s16 ws;                     /* +0x08 world structure (and visibility bit) */
    u8 kind;                    /* +0x0A */
    u8 limit;                   /* +0x0B */
    u8 period;                  /* +0x0C blink period in frames, 0 = steady */
    u8 count;                   /* +0x0D */
    u8 pad0e[2];
} Flare;

typedef struct {                /* 0x68 */
    u8 pad00[0x62];
    u8 r, g, b;                 /* +0x62 back colour */
    u8 pad65;
    u16 fog;                    /* +0x66 */
} FlareLight;

typedef struct {
    u8 *prim;                   /* +0x00 */
    u8 *end;                    /* +0x04 */
    u32 *ot;                    /* +0x08 */
    SVECTOR d;                  /* +0x0C position relative to the camera */
    SVECTOR n;                  /* +0x14 normal (4096, 0, 0) */
    MATRIX vm;                  /* +0x1C view rotation, translation from rtv0 */
    MATRIX cm;                  /* +0x3C camera rotation and position */
    s32 fog;                    /* +0x5C */
    s32 dp;                     /* +0x60 */
    s32 count;                  /* +0x64 */
    s16 cx, cy, cz;             /* +0x68 */
} FlareScr;

#define FSCR ((FlareScr *)0x1F800000)

typedef struct {
    u32 *ot;                    /* 0x801A5F28 */
    u8 pad04[0x78A0];
    MATRIX view;                /* 0x801AD7CC */
} OtView;

typedef struct {
    MATRIX m;                   /* 0x801AD67C */
    VECTOR pos;                 /* 0x801AD69C */
} CamMtx;

extern OtView D_801A5F28;
extern DrawCtx D_801A5F24;
typedef struct {
    u32 mask[0x41];             /* +0x000 0x801AC860 world-structure visibility bits */
    FlareLight lights[32];      /* +0x104 0x801AC964 */
    u8 pad[0xE3C - 0x104 - 32 * 0x68];
    s32 posx;                   /* +0xE3C 0x801AD69C camera position x */
    u8 pad2[0xFCC - 0xE40];
    Flare flares[256];          /* +0xFCC 0x801AD82C */
} FlareWorld;

extern MATRIX D_801AD67C;
extern FlareWorld D_801AC860;
extern s32 D_801AD6A0;
extern s32 D_801AD690;
extern s32 D_801AD694;
extern s32 D_801AD698;
extern s32 D_801AD6A4;
extern FlareDef *D_8008DC90[];
extern u32 D_801AC81C;
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);
extern s32 func_80061F44(s32 a, Flare *f, s32 w, s32 h);

/* draw the lamp / flare sprites (256 slots), lit by their world
 * structure's light set; blinking ones count their frames */
void func_80062090(s32 arg0)
{
    FlareScr *scr;
    Flare *e;
    FlareDef *def;
    FlareLight *ls;
    POLY_FT4 *p;
    s32 *src;
    s32 *dst;
    s32 k;
    s32 last;
    s32 otz;
    s32 w;
    s32 li;
    u32 f;

    last = -1;
    scr = FSCR;
    FSCR->ot = D_801A5F28.ot;
    FSCR->prim = D_801A5F24.buf->cur;
    FSCR->end = D_801A5F24.buf->end;
    src = (s32 *)&D_801A5F28.view;
    dst = (s32 *)&FSCR->vm;
    k = 5;
    do {
        *dst++ = *src++;
    } while (--k);
    src = (s32 *)&D_801AD67C;
    dst = (s32 *)&scr->cm;
    k = 5;
    do {
        *dst++ = *src++;
    } while (--k);
    scr->n.vx = 4096;
    scr->n.vz = 0;
    scr->n.vy = 0;
    scr->count = 256;
    scr->cm.t[0] = D_801AC860.posx;
    scr->cm.t[1] = D_801AD6A0;
    scr->cm.t[2] = D_801AD6A4;
    scr->cx = D_801AD690;
    scr->cy = D_801AD694;
    scr->cz = D_801AD698;
    e = D_801AC860.flares;
    do {
        if (e->id == -1) {
            goto next;
        }
        w = e->ws;
        if (!(D_801AC860.mask[w >> 5] & (1 << (w & 0x1F)))) {
            goto next;
        }
        gte_SetRotMatrix(&scr->cm);
        gte_SetTransMatrix(&scr->cm);
        if (func_80061F44(arg0, e, 500, 500) == 0) {
            goto next;
        }
        li = g_world_structs[e->ws].unk30[0];
        if (li != last) {
            last = li;
            ls = &D_801AC860.lights[last];
            SetBackColor(ls->r, ls->g, ls->b);
            f = ls->fog;
            scr->fog = f;
            if (f != 0) {
                SetFogNearFar((f * 2) >> 4, (f * 2) >> 2, 160);
            }
        }
        scr->d.vx = e->x - scr->cx;
        scr->d.vy = e->y - scr->cy;
        scr->d.vz = e->z - scr->cz;
        gte_ldv0(&scr->d);
        gte_rtv0();
        gte_stlvnl(scr->vm.t);
        gte_SetRotMatrix(&scr->vm);
        gte_SetTransMatrix(&scr->vm);
        p = (POLY_FT4 *)scr->prim;
        scr->prim = (u8 *)(p + 1);
        def = D_8008DC90[e->id] + e->kind + e->count;
        gte_ldv3(&def->v[0], &def->v[1], &def->v[2]);
        gte_rtpt();
        *(u32 *)&p->u0 = def->uv0;
        *(u32 *)&p->u1 = def->uv1;
        gte_stsxy3(&p->x0, &p->x1, &p->x2);
        gte_stszotz(&otz);
        if (otz != 0) {
            if (scr->fog == 0) {
                scr->dp = 0;
            } else {
                gte_stdp(&scr->dp);
            }
            gte_ldv0(&def->v[3]);
            gte_rtps();
            *(u16 *)&p->u2 = def->uv2;
            *(u16 *)&p->u3 = def->uv3;
            setlen(p, 9);
            gte_stsxy(&p->x3);
            gte_ldv0(&scr->n);
            gte_ldrgb(&def->rgb);
            gte_lddp(scr->dp);
            gte_ncds();
            gte_strgb(&p->r0);
            otz >>= 1;
            if (scr->d.vy <= 0) {
                otz += 120;
            } else {
                otz += 100;
            }
            addPrim(scr->ot + otz, p);
            if (e->period != 0 && D_801AC81C % e->period == 0) {
                if (++e->count >= e->limit) {
                    e->count = 0;
                }
            }
        } else {
            scr->prim -= sizeof(POLY_FT4);
        }
    next:
        e++;
    } while (--scr->count);
    D_801A5F24.buf->cur = scr->prim;
}
