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
    s16 model;                  /* +0x12 >= 0 drawn */
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
    u8 count;                   /* +0x05 parts */
} RecHdr;

typedef struct {                /* scratchpad 0x1F800000 */
    u8 pad00[0x38];
    MATRIX m;                   /* +0x38 model matrix */
    MATRIX m2;                  /* +0x58 part matrix */
    u8 pad78[0x20];
    VECTOR tmp;                 /* +0x98 part translation */
} DrawSpad2;

typedef struct {                /* 0x801A5F30 */
    u8 *model;                  /* +0x000 */
    u8 pad004[0x610];
    void *verts;                /* +0x614 (0x801A6544) */
} RenderGlobals;

extern MATRIX D_800B1228;
extern RenderGlobals D_801A5F30;
extern void func_8005804C(u16 part, s32 a, s32 lights);
extern void func_80059198(u16 part, s32 a, u16 b, s32 lights);

/* ovl202 0x80058B04 - draw every part of an AC-like object: each part's world matrix is
 * brought into camera space with the model matrix D_800B1228, then the part model
 * (index base + part->model) is drawn by func_8005804C, or func_80059198 when b != 0. */
void func_80058B04(PartObj *o, RecHdr *hdr, s32 a, u16 b, s32 lights, s32 base)
{
    DrawSpad2 *sp;
    Part7C *p;
    s32 n;
    s32 i;
    s32 idx;
    u32 *src;
    u32 *dst;

    sp = (DrawSpad2 *)0x1F800000;
    p = o->parts;
    n = hdr->count;
    src = (u32 *)&D_800B1228;
    dst = (u32 *)&sp->m;
    i = 5;
    do {
        *dst++ = *src++;
    } while (--i);
    sp->m.t[0] = D_800B1228.t[0];
    sp->m.t[1] = D_800B1228.t[1];
    sp->m.t[2] = D_800B1228.t[2];
    while (--n != -1) {
        if (p->model >= 0) {
            gte_SetRotMatrix(&sp->m);
            gte_SetTransMatrix(&sp->m);
            gte_ldlvl(p->world.t);
            gte_rt();
            gte_stlvnl(&sp->tmp);
            gte_ldclmv(&p->world.m[0][0]);
            gte_rtir();
            gte_stclmv(&sp->m2.m[0][0]);
            gte_ldclmv(&p->world.m[0][1]);
            gte_rtir();
            gte_stclmv(&sp->m2.m[0][1]);
            gte_ldclmv(&p->world.m[0][2]);
            gte_rtir();
            gte_stclmv(&sp->m2.m[0][2]);
            gte_SetRotMatrix(&sp->m2);
            gte_SetTransMatrix((MATRIX *)((u8 *)&sp->tmp - 20));
            idx = p->model + base;
            D_801A5F30.verts = *(void **)(D_801A5F30.model + (idx * 32 + 12) - idx * 4);
            if (b == 0) {
                func_8005804C(idx, a, (1 << p->model) & lights);
            } else {
                func_80059198(idx, a, b, (1 << p->model) & lights);
            }
        }
        p++;
    }
}
