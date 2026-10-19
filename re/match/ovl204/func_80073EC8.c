#include "ac1_types.h"
/* ovl204 0x80073EC8  size 276  score 7.166  2026-10-18
 * Apply damage to an object: the two player slots of D_801AB748 compute it
 * with a temporarily swapped table pointer, others from their definition.
 * HP (u16 at +0x160) is clamped at 0, then the hit callback runs. */

typedef struct {
    u8 pad[0x14];
    u16 h14;
} Func80073EC8Def;

typedef struct Func80073EC8Obj {
    Func80073EC8Def *def;                   /* +0x00 */
    u8 pad4[0x5C - 4];
    void (*hit)(struct Func80073EC8Obj *o, s32 a, s32 b, s32 c, s32 dmg); /* +0x5C */
    u8 pad60[0x160 - 0x60];
    u16 hp;                                 /* +0x160 */
    u8 pad162[0x170 - 0x162];
} Func80073EC8Obj;

extern Func80073EC8Obj D_801AB748[];
extern void *D_80039D18;
extern u8 D_80040FA0[];
extern u8 D_8004133C[];
extern s32 func_80073DE8(s32 a);
extern s32 func_80073E6C(u16 k, s32 a);

void func_80073EC8(Func80073EC8Obj *obj, s32 a, s32 b, s32 c)
{
    void *saved;
    s32 dmg;
    s32 hp;

    if (obj == &D_801AB748[0]) {
        saved = D_80039D18;
        D_80039D18 = D_80040FA0;
        dmg = func_80073DE8(a);
        D_80039D18 = saved;
    } else if (obj == &D_801AB748[1]) {
        saved = D_80039D18;
        D_80039D18 = D_8004133C;
        dmg = func_80073DE8(a);
        D_80039D18 = saved;
    } else {
        dmg = func_80073E6C(obj->def->h14, a);
    }
    dmg *= 4;
    hp = obj->hp - dmg;
    if (hp < 0) {
        obj->hp = 0;
    } else {
        obj->hp = hp;
    }
    if (obj->hit != 0) {
        obj->hit(obj, a, b, c, dmg);
    }
}
