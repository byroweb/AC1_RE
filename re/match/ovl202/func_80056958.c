#include "ac1/ac1.h"

typedef struct { s16 v[9]; } S16x9;
typedef struct { s16 v[10]; } S16x10;

typedef struct {                /* 0x2C */
    S16x10 b;                   /* +0x00 */
    S16x9 a;                    /* +0x14 */
    DVECTOR c;                  /* +0x26 */
    u16 d;                      /* +0x2A */
} SrcRec2C;

typedef struct {                /* 0x68 */
    S16x10 b;                   /* +0x00 */
    u8 pad14[0x3C];
    S16x9 a;                    /* +0x50 */
    DVECTOR c;                  /* +0x62 */
    u16 d;                      /* +0x66 */
} DstRec68;

extern SrcRec2C D_800B1248[];
extern DstRec68 D_801AC964[];

void func_80056958(void) {
    DstRec68 *d;
    SrcRec2C *s;
    s32 i;

    s = D_800B1248;
    d = D_801AC964;
    i = 31;
    do {
        d->a = s->a;
        d->b = s->b;
        d->c = s->c;
        d->d = s->d;
        s++;
        d++;
    } while (--i != -1);
}
