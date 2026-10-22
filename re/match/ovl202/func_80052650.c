#include "ac1/ac1.h"

typedef struct {
    s16 x0, y0;                 /* +0x00 */
    s16 x1, y1;                 /* +0x04 */
    s16 x2, y2;                 /* +0x08 */
    u8 c;                       /* +0x0C */
    u8 prio;                    /* +0x0D */
    u8 e, f;                    /* +0x0E */
    s32 id;                     /* +0x10 0 = free */
} ScrThread;

/* mission script threads at 0x801ED7A0 */
typedef struct {
    ScrThread th[8];            /* +0x000 */
    ScrThread *cur;             /* +0x0A0 (0x801ED840) */
    ScrThread *last;            /* +0x0A4 */
} ScrVM;

extern ScrVM D_801ED7A0;
extern void func_8005239C(s32 a);
extern ScrThread *func_800523BC(ScrThread *t);
extern void func_80052440(ScrThread *t);

/* queue a script window; prio -1 replaces the last one instead.  Refused
 * when the next slot is busy, or a live thread has the same id or a
 * higher priority. */
void func_80052650(s32 x, s32 y, s32 w, s32 h, s32 x2, s32 y2, s32 e, s32 f, s32 c, s32 prio,
                   s32 id)
{
    ScrThread *t;
    ScrThread *q;
    s32 i;

    if (prio != -1) {
        t = D_801ED7A0.cur;
        if (t->id != 0) {
            return;
        }
        q = D_801ED7A0.th;
        for (i = 8; i != 0; i--) {
            if (q->id != 0 && (q->id == id || prio < q->prio)) {
                return;
            }
            q++;
        }
    } else {
        t = D_801ED7A0.last;
    }
    t->x1 = x + w;
    t->y1 = y + h;
    t->x2 = x2 + x;
    t->y2 = y2 + y;
    t->x0 = x;
    t->y0 = y;
    t->e = e;
    t->f = f;
    t->c = c;
    t->id = id;
    if (prio != -1) {
        t->prio = prio;
        D_801ED7A0.cur = func_800523BC(t);
        if (t->prio != 0) {
            func_80052440(t);
        }
    } else {
        func_8005239C(0);
    }
}
