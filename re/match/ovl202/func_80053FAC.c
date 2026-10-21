#include "ac1/ac1.h"
#include "ac1/gte.h"

/* PsyQ inline_c.h shapes not in gte.h yet */
#define gte_lddp(r0) __asm__ volatile ( \
    "mtc2 %0, $8" : : "r"(r0))

#define gte_ldsv(r0) __asm__ volatile ( \
    "lhu $12, 0(%0)\n\t" \
    "lhu $13, 2(%0)\n\t" \
    "lhu $14, 4(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")

#define gte_gpf12() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B98003D")

#define gte_stsv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sh $12, 0(%0)\n\t" \
    "sh $13, 2(%0)\n\t" \
    "sh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct Part7C {
    u8 pad00[0x26];
    u16 flags;                  /* +0x26 bit 0 = local matrix stale */
    u8 pad28[0x24];
    s16 trans[3];               /* +0x4C */
    s16 rot[3];                 /* +0x52 */
    s16 scale[3];               /* +0x58 4096 = 1.0 */
    u8 pad5e[0x1E];
} Part7C;

typedef struct {                /* scratchpad 0x1F800000 */
    s32 n;                      /* +0x00 commands left */
    s16 v[3];                   /* +0x04 delta, scaled by the GTE */
} AnimSpad;

#define SPAD ((AnimSpad *)0x1F800000)

/* Relative part-animation commands (same stream as func_80053A54), each
 * delta weighted by w / 4096 through GPF. */
void func_80053FAC(u16 *p, Part7C *parts, s32 w, s32 mask) {
    Part7C *pt;
    s32 cmd;
    s32 c;
    AnimSpad *sp;

    sp = SPAD;
    gte_lddp(w);
    sp->n = *p++;
    while (--sp->n != -1) {
        cmd = *p++;
        pt = &parts[cmd >> 8];
        if (!(cmd & 1)) {
            goto skip;
        }
        if (pt->flags & mask) {
            goto skip;
        }
        c = cmd & 0xC;
        if (c != 0) {
            if (c == 4) {
                sp->v[0] = *p++;
                sp->v[1] = *p++;
                sp->v[2] = *p++;
            rot_apply:
                gte_ldsv(sp->v);
                gte_gpf12();
                gte_stsv(sp->v);
                pt->rot[0] += sp->v[0];
                pt->rot[1] += sp->v[1];
                pt->rot[2] += sp->v[2];
            } else if (c == 8) {
                sp->v[0] = ((s8 *)p)[0];
                sp->v[1] = ((s8 *)p)[1];
                sp->v[2] = ((s8 *)p)[2];
                p += 2;
                goto rot_apply;
            }
            pt->flags |= 1;
        }
        c = cmd & 0x30;
        if (c != 0) {
            if (c == 0x10) {
                pt->scale[0] = (pt->scale[0] * ((((s16)*p++ - 4096) * w + 4096) >> 12)) >> 12;
                pt->scale[1] = (pt->scale[1] * ((((s16)*p++ - 4096) * w + 4096) >> 12)) >> 12;
                pt->scale[2] = (pt->scale[2] * ((((s16)*p++ - 4096) * w + 4096) >> 12)) >> 12;
            } else if (c == 0x20) {
                pt->scale[0] = (pt->scale[0] * (((((s8 *)p)[0] - 4096) * w + 4096) >> 12)) >> 12;
                pt->scale[1] = (pt->scale[1] * (((((s8 *)p)[1] - 4096) * w + 4096) >> 12)) >> 12;
                pt->scale[2] = (pt->scale[2] * (((((s8 *)p)[2] - 4096) * w + 4096) >> 12)) >> 12;
                p += 2;
            }
            pt->flags |= 1;
        }
        c = cmd & 0xC0;
        if (c != 0) {
            if (c == 0x40) {
                sp->v[0] = *p++;
                sp->v[1] = *p++;
                sp->v[2] = *p++;
            trans_apply:
                gte_ldsv(sp->v);
                gte_gpf12();
                gte_stsv(sp->v);
                pt->trans[0] += sp->v[0];
                pt->trans[1] += sp->v[1];
                pt->trans[2] += sp->v[2];
            } else if (c == 0x80) {
                sp->v[0] = ((s8 *)p)[0];
                sp->v[1] = ((s8 *)p)[1];
                sp->v[2] = ((s8 *)p)[2];
                p += 2;
                goto trans_apply;
            }
        }
        continue;
    skip:
        c = cmd & 0xC;
        if (c != 0) {
            if (c == 4) {
                p += 3;
            } else if (c == 8) {
                p += 2;
            }
        }
        c = cmd & 0x30;
        if (c != 0) {
            if (c == 0x10) {
                p += 3;
            } else if (c == 0x20) {
                p += 2;
            }
        }
        c = cmd & 0xC0;
        if (c != 0) {
            if (c == 0x40) {
                p += 3;
            } else if (c == 0x80) {
                p += 2;
            }
        }
    }
}
