#include "ac1/ac1.h"
#include "ac1/gte.h"

#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")

/* ovl204: perspective-transform n vertices of D_801AE2EC into D_801AE340
 * (screen xy, otz, depth cue and a clip code), pipelined through the GTE. */
typedef struct {
    s32 sxy;            /* +0x00 */
    s16 otz;            /* +0x04 */
    u8 clip;            /* +0x06 */
    u8 pad7[5];
    s32 p;              /* +0x0C */
} PVtx;
typedef struct {
    u8 pad0[0x0C];
    u16 x, y;           /* +0x0C sxy */
    s32 p;              /* +0x10 */
    s32 otz;            /* +0x14 */
} Scr4;
#define SCR ((Scr4 *)0x1F800000)

extern PVtx D_801AE340[];
extern SVECTOR *D_801AE2EC;
extern void func_800594D4(SVECTOR *v, void *sxy, s32 *p, s32 *otz);

void func_80059508(s32 n)
{
    PVtx *o = D_801AE340;
    SVECTOR *v = D_801AE2EC;
    Scr4 *s = SCR;

    func_800594D4(v, &SCR->x, &SCR->p, &SCR->otz);
    v++;
    while (--n != 0) {
        gte_ldv0(v);
        gte_rtps();
        o->p = s->p;
        o->otz = s->otz;
        o->sxy = *(s32 *)&s->x;
        if ((s16)s->x < 0) {
            o->clip = 1;
        } else if ((s16)s->x >= 159) {
            o->clip = 2;
        } else {
            o->clip = 0;
        }
        if ((s16)s->y < 0) {
            o->clip |= 4;
        } else if ((s16)s->y >= 121) {
            o->clip |= 8;
        }
        v++;
        o++;
        gte_stsxy(&s->x);
        gte_stdp(&s->p);
        gte_stszotz(&s->otz);
    }
    o->p = s->p;
    o->otz = s->otz;
    o->sxy = *(s32 *)&s->x;
    if ((s16)s->x < 0) {
        o->clip = 1;
    } else if ((s16)s->x >= 159) {
        o->clip = 2;
    } else {
        o->clip = 0;
    }
    if ((s16)s->y < 0) {
        o->clip |= 4;
    } else if ((s16)s->y >= 121) {
        o->clip |= 8;
    }
}
