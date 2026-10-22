#include "ac1/ac1.h"

typedef struct {
    s16 a;
    s16 b;
    s16 c;
    s16 d;
} StrmEnt;

typedef struct {
    s32 buf0;           /* +0x00 */
    s32 buf1;           /* +0x04 */
    s32 cur;            /* +0x08 */
    s32 bufc;           /* +0x0C */
    s32 buf10;          /* +0x10 */
    s32 w14;            /* +0x14 */
    StrmEnt ent[2];     /* +0x18 */
    s32 sel;            /* +0x28 */
    s16 h2c;            /* +0x2C */
    s16 h2e;            /* +0x2E */
    s16 h30;            /* +0x30 */
    s16 pad32;
    s32 done;           /* +0x34 */
} Strm;

extern s32 D_8017F438;
extern s32 D_8017F43C;
extern s32 D_8017F440;
extern s32 D_8017F444;

void func_800A499C(Strm *s, s32 a, s32 b, s32 c, s32 d)
{
    s->cur = 0;
    s->w14 = 0;
    s->ent[0].a = a;
    s->ent[0].b = b;
    s->ent[1].a = c;
    s->sel = 0;
    s->h2c = a;
    s->h2e = b;
    s->h30 = 24;
    s->done = 0;
    s->buf0 = D_8017F438;
    s->buf1 = D_8017F43C;
    s->bufc = D_8017F440;
    s->buf10 = D_8017F444;
    s->ent[1].b = d;
}
