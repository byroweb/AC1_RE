#include "ac1_types.h"
/* ovl203 0x8007408C  size 268 */
typedef struct {
    u8 pad[0x14];
    u16 kind;           /* +0x14 */
} Func8007408CDef;
typedef struct Func8007408CObj Func8007408CObj;
struct Func8007408CObj {
    Func8007408CDef *def;   /* +0x000 */
    u8 pad4[0x5C - 4];
    void (*cb)(Func8007408CObj *, int, int, int, int); /* +0x05C */
    u8 pad60[0x160 - 0x60];
    u16 hp;             /* +0x160 */
};
typedef struct {
    u8 pad[0x394];
    Func8007408CObj *obj;   /* +0x394 */
} Func8007408CPlayer;
extern Func8007408CPlayer *D_80039D30;
extern Func8007408CPlayer *D_80039D34;
extern Func8007408CPlayer *D_80039D18;
extern int func_80073FAC(int);
extern int func_80074030(int, int);

void func_8007408C(Func8007408CObj *o, int a1, int a2, int a3)
{
    Func8007408CPlayer *saved;
    int dmg;
    int hp;

    if (o == D_80039D30->obj) {
        saved = D_80039D18;
        D_80039D18 = D_80039D30;
        dmg = func_80073FAC(a1);
        D_80039D18 = saved;
    } else if (o == D_80039D34->obj) {
        saved = D_80039D18;
        D_80039D18 = D_80039D34;
        dmg = func_80073FAC(a1);
        D_80039D18 = saved;
    } else {
        dmg = func_80074030(o->def->kind, a1);
    }
    dmg *= 4;
    hp = o->hp - dmg;
    if (hp < 0) {
        o->hp = 0;
    } else {
        o->hp = hp;
    }
    if (o->cb != 0) {
        o->cb(o, a1, a2, a3, dmg);
    }
}
