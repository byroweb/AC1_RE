#include "ac1/ac1.h"
/* PROVISIONAL MATCH (wave7 a1): register asm("$4") pin + empty asm barrier order a0=0 before the *lp store; rest is plain C. */
/* ovl202 0x80052440 - mission script VM: stop every thread of lower priority than me,
 * pack the surviving threads towards the ring start, then move the ring start to the
 * first live thread and the current thread to the first free slot after it. */

typedef struct {
    u8 unk0[0xD];
    u8 pri;                     /* +0x0D */
    u8 unkE[2];
    s32 state;                  /* +0x10 */
} ScrThread;

typedef struct {
    ScrThread th[8];            /* +0x000 */
    ScrThread *cur;             /* +0x0A0 (0x801ED840) */
    ScrThread *last;            /* +0x0A4 */
    u8 unka8[8];
    u8 *blob;                   /* +0x0B0 */
} ScrVM;

extern ScrVM D_801ED7A0;

ScrThread *func_800523BC(ScrThread *t);
void func_8005239C(s16 a);

void func_80052440(ScrThread *me)
{
    ScrThread *t;
    ScrThread *u;
    ScrThread *v;
    ScrThread *w;
    s32 killed;
    s32 gap;
    ScrThread **lp;

    t = D_801ED7A0.last;
    killed = 0;
    if (t->state != 0) {
        do {
            if (t->pri < me->pri) {
                t->state = 0;
                killed = 1;
            }
            t = func_800523BC(t);
            if (t == D_801ED7A0.last) {
                break;
            }
        } while (t->state != 0);
    }
    if (killed) {
        gap = 0;
        v = func_800523BC(D_801ED7A0.last);
        while (v != D_801ED7A0.last) {
            if (v->state != 0) {
                if (gap) {
                    u = func_800523BC(D_801ED7A0.last);
                    if (u != v) {
                        while (1) {
                        top:
                            if (u->state == 0) {
                                goto found;
                            }
                            u = func_800523BC(u);
                            if (u != v) {
                                goto top;
                            }
                            goto out;
                        }
                    found:
                        *u = *v;
                        v->state = 0;
                    }
                }
            } else {
                gap = 1;
            }
        out:
            v = func_800523BC(v);
        }
    }
    if (D_801ED7A0.last->state == 0) {
        w = func_800523BC(D_801ED7A0.last);
        lp = &D_801ED7A0.last;
        do {
            if (w->state != 0) {
                {
                    register s32 z asm("$4") = 0;
                    __asm__ volatile("" : : "r"(z));
                    *lp = w;
                    func_8005239C(z);
                }
                goto test;
            body:
                w = func_800523BC(w);
                if (w == *lp) {
                    return;
                }
            test:
                if (w->state != 0) {
                    goto body;
                }
                lp[-1] = w;
                return;
            }
            w = func_800523BC(w);
        } while (w != D_801ED7A0.last);
    }
}
