#include "ac1/ac1.h"

#define gte_ldlvl(r0) __asm__ volatile ("lwc2 $9, 0(%0)\n\tlwc2 $10, 4(%0)\n\tlwc2 $11, 8(%0)" : : "r"(r0))
#define gte_sqr12() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AA00428")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")
#define gte_ldlzc(r0) __asm__ volatile ("mtc2 %0, $30\n\tnop" : : "r"(r0))
#define gte_stlzc(r0) __asm__ volatile ("swc2 $31, 0(%0)" : : "r"(r0) : "memory")
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ldcv(r0) __asm__ volatile ("lbu $12, 0(%0)\n\tlbu $13, 1(%0)\n\tlbu $14, 2(%0)\n\tmtc2 $12, $9\n\tmtc2 $13, $10\n\tmtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_gpf12() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B98003D")
#define gte_gpl12() __asm__ volatile ("nop\n\tnop\n\t.word 0x4BA8003E")
#define gte_stcv(r0) __asm__ volatile ("mfc2 $12, $9\n\tmfc2 $13, $10\n\tmfc2 $14, $11\n\tsb $12, 0(%0)\n\tsb $13, 1(%0)\n\tsb $14, 2(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct {                /* 0x20 point light */
    s16 x, y, z;                /* +0x00 */
    u8 pad06[0x12];
    u8 col[4];                  /* +0x18 */
    s16 radius;                 /* +0x1C */
    s16 pad1e;
} PtLight;

typedef struct {
    u8 pad00[0x10];
    struct { u8 pad[0x18]; SVECTOR *pos; } *obj;   /* +0x10 */
    u8 pad14[0x18];
    VECTOR wpos;                /* +0x2C (x,y,z) */
    VECTOR d;                   /* +0x3C */
    VECTOR sq;                  /* +0x4C */
    PtLight light[4];           /* +0x5C */
    s32 n_light;                /* +0xDC */
    u8 col[4];                  /* +0xE0 */
} LightScr;

#define LSCR ((LightScr *)0x1F800000)

extern u16 D_80031564[];
extern void func_800672A4(SVECTOR *v, VECTOR *out);

/* ambient 0x40 plus the point lights' colours, attenuated linearly by
 * distance / radius, into the scratchpad colour */
void func_800672CC(void)
{
    PtLight *l;
    s32 n;
    s32 sum;
    s32 lz;
    s32 sh;
    s32 t;
    LightScr *sc;
    u8 *c;
    u16 *tb;

    sc = LSCR;
    func_800672A4(LSCR->obj->pos, &LSCR->wpos);
    *(u32 *)LSCR->col = 0x404040;
    l = LSCR->light;
    tb = D_80031564;
    c = LSCR->col;
    n = LSCR->n_light;
    do {
        sc->d.vx = sc->wpos.vx - l->x;
        sc->d.vy = sc->wpos.vy - l->y;
        sc->d.vz = sc->wpos.vz - l->z;
        gte_ldlvl(&sc->d);
        gte_sqr12();
        gte_stlvnl(&sc->sq);
        sum = sc->sq.vx + sc->sq.vy + sc->sq.vz;
        gte_ldlzc(sum);
        gte_stlzc(&lz);
        lz &= ~1;
        t = lz - 24;
        sh = (31 - lz) >> 1;
        if (t <= 0) {
            t = sum >> (24 - lz);
        } else {
            t = sum << t;
        }
        sum = tb[t] << sh;
        sum = sum / l->radius;
        if (sum > 4096) {
            sum = 4096;
        }
        gte_lddp(4096 - sum);
        gte_ldcv(l->col);
        gte_gpf12();
        gte_lddp(sum);
        gte_ldcv(c);
        gte_gpl12();
        gte_stcv(c);
        l++;
    } while (--n);
}
