#include "ac1/ac1.h"
/* ovl202 0x800563DC - draw all n parts of a RecSet with the scratchpad model matrix
 * (the o == 0 path of func_800564AC): count the primitives, reserve them
 * (func_80056304), load the matrix into the GTE, draw each part (func_800560F4). */

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

typedef struct {                /* 0x1C */
    u8 pad00[0x14];
    u16 nprim;                  /* +0x14 */
    u8 pad16[6];
} Rec1C;

typedef struct {
    MATRIX m;                   /* +0x00 model matrix */
    MATRIX m2;                  /* +0x20 */
    VECTOR tmp;                 /* +0x40 */
    void *hdr;                  /* +0x50 */
    Rec1C *set;                 /* +0x54 RecSet, then its rec[] */
} DrawSpad;

extern void func_80056304(DrawSpad *, s32);
extern void func_800560F4(s32);

void func_800563DC(DrawSpad *sp, s32 n)
{
    Rec1C *r;
    s32 total;
    s32 i;

    total = 0;
    r = sp->set = (Rec1C *)((u8 *)sp->set + 12);
    for (i = n - 1; i != 0; i--) {
        total += r->nprim;
        r++;
    }
    func_80056304(sp, total);
    gte_SetRotMatrix(&sp->m);
    gte_SetTransMatrix(&sp->m);
    for (i = 0; i < n; i++) {
        func_800560F4(i);
    }
}
