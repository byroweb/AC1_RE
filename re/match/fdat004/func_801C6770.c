#include "ac1/ac1.h"

typedef struct {
    u8 pad0[8];
    u8 cnt;             /* +0x08 */
} Ctr;
typedef struct {
    s16 pos[3];         /* +0x00 */
    u8 pad6[8];
    u16 idx;            /* +0x0E */
    u8 pad10[4];
    Ctr *ctr;           /* +0x14 */
    u8 pad18[4];
    u16 flags;          /* +0x1C */
    u8 pad1e[0x0E];
} Slot2C;               /* 0x2C */

extern s32 rand(void);
extern s32 SquareRoot0(s32);
extern Projectile *func_80086F20(s32, Slot2C *, s32, s32, s32, void *, void *, void *, void *);
extern Projectile *func_80086854(s32, Slot2C *, void *);
extern u32 D_801AC860[];
extern s16 D_801A26C0;
extern s16 D_801A26C2;
extern s16 D_801A26C4;
extern u8 D_8019F52E[];
extern u8 D_801C7204[];
extern u8 D_801C720C[];
extern u8 D_801C7210[];
extern u8 D_801C7214[];

void func_801C6770(Slot2C *s)
{
    Ctr *c;
    Projectile *p;
    s32 dx;
    s32 dy;
    s32 dz;

    if (!(s->flags & 0x8000)) {
        c = s->ctr;
        if (!(c->cnt & 7)) {
            if (D_801AC860[(s16)s->idx >> 5] & (1 << (s->idx & 31))) {
                dx = D_801A26C0 - s->pos[0];
                dy = D_801A26C2 - s->pos[1];
                dz = D_801A26C4 - s->pos[2];
                if (SquareRoot0(dx * dx + dy * dy + dz * dz) < 12000) {
                    func_80086F20(0, s, 41, 80, (rand() & 3) + 6, D_801C7204, D_8019F52E, D_801C720C, D_801C7210)->unk12 = 80;
                    p = func_80086854(0, s, D_801C7214);
                    p->kind = 5;
                    p->ang[2] = 400;
                    p->ang[1] = 400;
                    p->unkbe = 100;
                }
            }
        }
        c->cnt++;
    }
}
