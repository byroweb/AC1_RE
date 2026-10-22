#include "ac1/ac1.h"
#include "ac1/gte.h"

/* PsyQ inline_c.h shapes not in gte.h yet */
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

#define gte_rt() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A480012")

#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25, 0(%0)\n\t" \
    "swc2 $26, 4(%0)\n\t" \
    "swc2 $27, 8(%0)" : : "r"(r0) : "memory")

#define gte_ldclmv(r0) __asm__ volatile ( \
    "lhu $12, 0(%0)\n\t" \
    "lhu $13, 6(%0)\n\t" \
    "lhu $14, 12(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")

#define gte_rtir() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A49E012")

#define gte_stclmv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sh $12, 0(%0)\n\t" \
    "sh $13, 6(%0)\n\t" \
    "sh $14, 12(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

#define gte_ldlvl(r0) __asm__ volatile ( \
    "lhu $13, 4(%0)\n\t" \
    "lhu $12, 0(%0)\n\t" \
    "sll $13, $13, 16\n\t" \
    "or $12, $12, $13\n\t" \
    "mtc2 $12, $0\n\t" \
    "lwc2 $1, 8(%0)" : : "r"(r0) : "$12", "$13")


typedef struct Part7C {
    u8 pad00[0x12];
    s16 model;                  /* +0x12 */
    u8 pad14[0x14];
    MATRIX world;               /* +0x28 */
    u8 pad48[0x34];
} Part7C;

typedef struct {
    u8 pad00[0x20];
    Part7C *parts;              /* +0x20 */
} PartObj;

typedef struct {                /* 0x1C */
    u8 pad00[0x18];
    u8 *verts;                  /* +0x18 */
} Rec1C;

typedef struct {
    u8 pad00[8];
    s32 off;                    /* +0x08 offset of the RecSet */
} RecHdr;

extern RecHdr *D_801A5F34[];
extern void func_80013AFC(s16 ang, MATRIX *m);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);

/* ovl202 0x80055A24 - transform n vertices (indices idx[], pair k of model `sub` of slot)
 * of a part (o != 0: in the part's world frame, k = 1) or of a free model (k = 2) at
 * pos/rot into out[]; returns 1 */
s32 func_80055A24(PartObj *o, s32 slot, SVECTOR *pos, SVECTOR *rot, SVECTOR *out, s32 part,
                  s32 sub, s32 n, s32 *idx)
{
    VECTOR tmp;
    MATRIX m;
    VECTOR r;
    Part7C *p;
    u8 *verts;
    s32 k;
    u8 *v;

    verts = ((Rec1C *)((u8 *)D_801A5F34[slot] + D_801A5F34[slot]->off + (sub * 32 + 12) - sub * 4))->verts;
    if (o != 0) {
        p = &o->parts[part];
        func_80013AFC(2048 - rot->vy, &m);
        m.m[0][0] >>= 2;
        m.m[0][2] >>= 2;
        m.m[1][1] >>= 2;
        m.m[2][0] >>= 2;
        m.m[2][2] >>= 2;
        m.t[0] = pos->vx;
        m.t[1] = pos->vy;
        m.t[2] = pos->vz;
        SetRotMatrix(&m);
        SetTransMatrix(&m);
        gte_ldlvl(p->world.t);
        gte_rt();
        gte_stlvnl(&tmp);
        gte_ldclmv(&p->world.m[0][0]);
        gte_rtir();
        gte_stclmv(&m.m[0][0]);
        gte_ldclmv(&p->world.m[0][1]);
        gte_rtir();
        gte_stclmv(&m.m[0][1]);
        gte_ldclmv(&p->world.m[0][2]);
        gte_rtir();
        gte_stclmv(&m.m[0][2]);
        SetRotMatrix(&m);
        SetTransMatrix((MATRIX *)((u8 *)&tmp - 20));
        k = 1;
    } else {
        func_80013AFC(2048 - rot->vy, &m);
        m.m[0][0] >>= 2;
        m.m[0][2] >>= 2;
        m.m[1][1] >>= 2;
        m.m[2][0] >>= 2;
        m.m[2][2] >>= 2;
        m.t[0] = pos->vx;
        m.t[1] = pos->vy;
        m.t[2] = pos->vz;
        SetRotMatrix(&m);
        SetTransMatrix(&m);
        k = 2;
    }
    while (--n != -1) {
        v = verts + (*idx++ << 4) + k * 8;
        gte_ldv0(v);
        gte_rt();
        gte_stlvnl(&r);
        out->vx = r.vx;
        out->vy = r.vy;
        out->vz = r.vz;
        out++;
    }
    return 1;
}
