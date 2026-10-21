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

typedef struct Part7C {
    u8 pad00[0x12];
    s16 ang;                    /* +0x12 */
    s16 rot[3][3];              /* +0x14 local rotation */
    u16 flags;                  /* +0x26 bit 1 = world matrix stale, bit 5 = no parent rotation */
    MATRIX world;               /* +0x28 */
    struct Part7C *parent;      /* +0x48 */
    SVECTOR trans;              /* +0x4C local translation */
    u8 pad54[0x28];
} Part7C;

void func_800543F0(Part7C *p) {
    Part7C *parent;

    if (p->flags & 2) {
        parent = p->parent;
        if (parent == 0) {
            p->world.t[0] = p->trans.vx;
            p->world.t[1] = p->trans.vy;
            p->world.t[2] = p->trans.vz;
            /* the 20 bytes of rot + flags, as words */
            {
                s32 *src = (s32 *)p->rot;
                s32 *dst = (s32 *)&p->world;
                s32 i;

                for (i = 5; i != 0; i--) {
                    *dst++ = *src++;
                }
            }
        } else {
            func_800543F0(parent);
            gte_SetRotMatrix(&parent->world);
            gte_SetTransMatrix(&parent->world);
            gte_ldv0(&p->trans);
            gte_rt();
            gte_stlvnl(p->world.t);
            if (p->flags & 0x20) {
                {
                    s32 *src = (s32 *)p->rot;
                    s32 *dst = (s32 *)&p->world;
                    s32 i;

                    for (i = 5; i != 0; i--) {
                        *dst++ = *src++;
                    }
                }
            } else {
                gte_ldclmv(&p->rot[0][0]);
                gte_rtir();
                gte_stclmv(&p->world.m[0][0]);
                gte_ldclmv(&p->rot[0][1]);
                gte_rtir();
                gte_stclmv(&p->world.m[0][1]);
                gte_ldclmv(&p->rot[0][2]);
                gte_rtir();
                gte_stclmv(&p->world.m[0][2]);
            }
        }
        p->flags &= ~2;
    }
}
