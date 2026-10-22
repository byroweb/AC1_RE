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

typedef struct {
    u8 pad00[5];
    u8 count;                   /* +0x05 */
} PartHdr;

typedef struct {                /* 0x1C */
    u32 *vert;                  /* +0x00 */
    u8 pad04[0x10];
    u16 n_prim;                 /* +0x14 */
    u8 pad16[6];
} MdlObj;

typedef struct {
    u32 w[5];
} Rot20;

typedef struct {                /* scratchpad 0x1F800000 */
    u8 pad00[0x38];
    MATRIX m;                   /* +0x38 camera */
    MATRIX m2;                  /* +0x58 */
    MATRIX w;                   /* +0x78 flattened part rotation */
    VECTOR tmp;                 /* +0x98 */
    VECTOR pos;                 /* +0xA8 */
} ShScr;

typedef struct {
    u8 *model;                  /* 0x8019D968 */
    u8 pad04[0x610];
    u32 *verts;                 /* 0x8019DF7C */
} RenderGlobals;

extern MATRIX D_800AD190;
extern RenderGlobals D_8019D968;
extern void func_800573FC(MdlObj *obj, s32 ot);

void func_80057640(PartObj *o, PartHdr *h, s32 ot)
{
    ShScr *sp;
    Part7C *p;
    MdlObj *obj;
    s32 n;
    s32 base;
    s32 idx;
    u8 *mdl;
    u32 *d;
    u32 *src;
    s32 i;

    sp = (ShScr *)0x1F800000;
    p = o->parts;
    n = h->count;
    mdl = D_8019D968.model;
    src = (u32 *)&D_800AD190;
    d = (u32 *)&sp->m;
    i = 5;
    do {
        *d++ = *src++;
    } while (--i != 0);
    sp->w.m[1][0] = sp->w.m[1][1] = sp->w.m[1][2] = 0;
    sp->pos.vy = 0;
    sp->m.t[0] = D_800AD190.t[0];
    sp->m.t[1] = D_800AD190.t[1];
    sp->m.t[2] = D_800AD190.t[2];
    base = *(s32 *)mdl;
    while (--n != -1) {
        idx = p->model;
        if (idx >= 0) {
            obj = (MdlObj *)(D_8019D968.model + ((idx + base) * 32 + 12) - (idx + base) * 4);
            if (obj->n_prim != 0) {
                gte_SetRotMatrix(&sp->m);
                gte_SetTransMatrix(&sp->m);
                sp->pos.vx = p->world.t[0];
                sp->pos.vz = p->world.t[2];
                gte_ldlvl(&sp->pos);
                gte_rt();
                gte_stlvnl(&sp->tmp);
                sp->w.m[0][0] = p->world.m[0][0];
                sp->w.m[0][1] = p->world.m[0][1];
                sp->w.m[0][2] = p->world.m[0][2];
                sp->w.m[2][0] = p->world.m[2][0];
                sp->w.m[2][1] = p->world.m[2][1];
                sp->w.m[2][2] = p->world.m[2][2];
                gte_ldclmv(&sp->w.m[0][0]);
                gte_rtir();
                gte_stclmv(&sp->m2.m[0][0]);
                gte_ldclmv(&sp->w.m[0][1]);
                gte_rtir();
                gte_stclmv(&sp->m2.m[0][1]);
                gte_ldclmv(&sp->w.m[0][2]);
                gte_rtir();
                gte_stclmv(&sp->m2.m[0][2]);
                gte_SetRotMatrix(&sp->m2);
                gte_SetTransMatrix((MATRIX *)((u8 *)&sp->tmp - 20));
                D_8019D968.verts = obj->vert;
                func_800573FC(obj, ot);
            }
        }
        p++;
    }
}
