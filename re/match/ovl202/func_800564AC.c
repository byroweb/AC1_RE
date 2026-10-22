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
    s16 model;                  /* +0x12 >= 0 drawn; < -2048: model + 4096, drawn */
    u8 pad14[0x14];
    MATRIX world;               /* +0x28 */
    u8 pad48[0x34];
} Part7C;

typedef struct {
    u8 pad00[0x20];
    Part7C *parts;              /* +0x20 */
} PartObj;

typedef struct {                /* 0x1C */
    u8 pad00[0x14];
    u16 nprim;                  /* +0x14 */
    u8 pad16[6];
} Rec1C;

typedef struct {
    u32 n;
    u32 unk04;
    u32 total;
    Rec1C rec[1];               /* +0x0C */
} RecSet;

typedef struct {
    u8 pad00[5];
    u8 count;                   /* +0x05 parts */
    u8 pad06[2];
    s32 off;                    /* +0x08 offset of the RecSet */
} RecHdr;

typedef struct {
    u8 *cur;                    /* 0x801A5F30 */
    RecHdr *tbl[1];             /* 0x801A5F34 */
} TexSlots;

typedef struct {                /* scratchpad 0x1F800000 */
    MATRIX m;                   /* +0x00 model matrix */
    MATRIX m2;                  /* +0x20 part matrix */
    VECTOR tmp;                 /* +0x40 part translation */
    RecHdr *hdr;                /* +0x50 */
    void *set;                  /* +0x54 RecSet, then its rec[] */
    u8 pad58[0x28];
    s32 f80;                    /* +0x80 */
    s32 counter;                /* +0x84 */
    void *out;                  /* +0x88 */
} DrawSpad;

#define SPAD ((DrawSpad *)0x1F800000)

extern TexSlots D_801A5F30;
extern void func_80013AFC(s16 ang, MATRIX *m);
extern void func_80056304(DrawSpad *, s32);
extern void func_800563DC(DrawSpad *, s32);
extern void func_800560F4(s32);

s32 func_800564AC(PartObj *o, s32 slot, SVECTOR *pos, SVECTOR *rot, s32 f80, void *out) {
    Part7C *p;
    RecSet *set;
    s32 n;
    s32 total;
    s32 idx;
    DrawSpad *sp;

    sp = SPAD;
    SPAD->hdr = D_801A5F30.tbl[slot];
    if (SPAD->hdr == 0) {
        return 0;
    }
    SPAD->f80 = f80;
    SPAD->out = out;
    SPAD->counter = 0;
    if (SPAD->hdr->count != 0) {
        func_80013AFC(2048 - rot->vy, &SPAD->m);
        SPAD->m.m[0][0] >>= 2;
        SPAD->m.m[0][2] >>= 2;
        SPAD->m.m[1][1] >>= 2;
        SPAD->m.m[2][0] >>= 2;
        SPAD->m.m[2][2] >>= 2;
        SPAD->m.t[0] = pos->vx;
        SPAD->m.t[1] = pos->vy;
        SPAD->m.t[2] = pos->vz;
        set = (RecSet *)((u8 *)SPAD->hdr + SPAD->hdr->off);
        SPAD->set = set;
        if (o == 0) {
            func_800563DC(sp, set->n);
            return 1;
        }
        SPAD->set = set->rec;
        total = 0;
        n = SPAD->hdr->count;
        p = o->parts;
        while (--n != -1) {
            idx = p->model;
            if (idx >= 0) {
                u16 *np;
            count:
                np = &((Rec1C *)sp->set)[idx].nprim;
                total += *np;
            } else if (idx < -2048) {
                idx += 4096;
                goto count;
            }
            p++;
        }
        func_80056304(sp, total);
        p = o->parts;
        n = sp->hdr->count;
        while (--n != -1) {
            idx = p->model;
            if (idx >= 0) {
            draw:
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
                func_800560F4(idx);
            } else if (idx < -2048) {
                idx += 4096;
                goto draw;
            }
            p++;
        }
        return 1;
    }
    func_80013AFC(2048 - rot->vy, &SPAD->m);
    SPAD->m.m[0][0] >>= 2;
    SPAD->m.m[0][2] >>= 2;
    SPAD->m.m[1][1] >>= 2;
    SPAD->m.m[2][0] >>= 2;
    SPAD->m.m[2][2] >>= 2;
    SPAD->m.t[0] = pos->vx;
    SPAD->m.t[1] = pos->vy;
    SPAD->m.t[2] = pos->vz;
    SPAD->set = (RecSet *)((u8 *)SPAD->hdr + SPAD->hdr->off);
    func_800563DC(sp, 1);
    return 1;
}
