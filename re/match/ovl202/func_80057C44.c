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

typedef struct {                /* 0x1C model record */
    void *verts;                /* +0x00 */
    u8 pad04[0x10];
    u16 npkt;                   /* +0x14 */
} PktModel;

typedef struct {
    s32 base;                   /* +0x00 first record of the parts */
    u32 flags;                  /* +0x04 bit 0: casts a shadow */
} ModelBody;

typedef struct {                /* scratchpad 0x1F800000 */
    u8 pad00[0x38];
    MATRIX m;                   /* +0x38 model matrix */
    MATRIX m2;                  /* +0x58 part matrix */
    MATRIX m3;                  /* +0x78 flattened part rotation */
    VECTOR tmp;                 /* +0x98 part translation */
    VECTOR t3;                  /* +0xA8 flattened part position */
} DrawSpad3;

typedef struct {                /* 0x801A5F30 */
    ModelBody *model;           /* +0x000 */
    u8 pad004[0x610];
    void *verts;                /* +0x614 (0x801A6544) */
} RenderGlobals;

extern MATRIX D_800B1228;
extern RenderGlobals D_801A5F30;
extern void func_80057A00(PktModel *m, s32 z);

/* ovl202 0x80057C44 - draw the shadow of every part: the part's world matrix with its
 * y row and y translation zeroed, through the model matrix, as flat grey quads
 * (func_80057A00); only for models with flag bit 0. */
void func_80057C44(PartObj *o, RecHdr *hdr, s32 z)
{
    DrawSpad3 *sp;
    Part7C *p;
    s32 n;
    s32 i;
    s32 idx;
    s32 base;
    PktModel *rec;
    ModelBody *mb;
    u32 *src;
    u32 *dst;

    mb = D_801A5F30.model;
    sp = (DrawSpad3 *)0x1F800000;
    n = hdr->count;
    p = o->parts;
    if (!(mb->flags & 1)) {
        return;
    }
    src = (u32 *)&D_800B1228;
    dst = (u32 *)&sp->m;
    i = 5;
    do {
        *dst++ = *src++;
    } while (--i);
    sp->m3.m[1][0] = sp->m3.m[1][1] = sp->m3.m[1][2] = 0;
    sp->t3.vy = 0;
    sp->m.t[0] = D_800B1228.t[0];
    sp->m.t[1] = D_800B1228.t[1];
    sp->m.t[2] = D_800B1228.t[2];
    base = mb->base;
    while (--n != -1) {
        if (p->model >= 0) {
            idx = p->model + base;
            rec = (PktModel *)((u8 *)D_801A5F30.model + (idx * 32 + 12) - idx * 4);
            if (rec->npkt != 0) {
                gte_SetRotMatrix(&sp->m);
                gte_SetTransMatrix(&sp->m);
                sp->t3.vx = p->world.t[0];
                sp->t3.vz = p->world.t[2];
                gte_ldlvl(&sp->t3);
                gte_rt();
                gte_stlvnl(&sp->tmp);
                sp->m3.m[0][0] = p->world.m[0][0];
                sp->m3.m[0][1] = p->world.m[0][1];
                sp->m3.m[0][2] = p->world.m[0][2];
                sp->m3.m[2][0] = p->world.m[2][0];
                sp->m3.m[2][1] = p->world.m[2][1];
                sp->m3.m[2][2] = p->world.m[2][2];
                gte_ldclmv(&sp->m3.m[0][0]);
                gte_rtir();
                gte_stclmv(&sp->m2.m[0][0]);
                gte_ldclmv(&sp->m3.m[0][1]);
                gte_rtir();
                gte_stclmv(&sp->m2.m[0][1]);
                gte_ldclmv(&sp->m3.m[0][2]);
                gte_rtir();
                gte_stclmv(&sp->m2.m[0][2]);
                gte_SetRotMatrix(&sp->m2);
                gte_SetTransMatrix((MATRIX *)((u8 *)&sp->tmp - 20));
                D_801A5F30.verts = rec->verts;
                func_80057A00(rec, z);
            }
        }
        p++;
    }
}
