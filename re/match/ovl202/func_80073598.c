#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x1E];
    u16 w;              /* +0x1E */
    u16 h;              /* +0x20 */
} HitObj;

extern HitObj *D_8019F308[];
extern s16 D_8019F508;
extern s32 func_80012E5C(s32, s32, s32, HitObj *, s32, s32);

HitObj *func_80073598(HitObj *self, s32 a, s32 b, s32 c)
{
    HitObj **pp;
    HitObj *o;
    s32 i;

    pp = D_8019F308;
    i = D_8019F508;
    while (--i != -1) {
        o = *pp++;
        if (o != self) {
            if (func_80012E5C(a, b, c, o, o->w, o->h) != 0) {
                return o;
            }
        }
    }
    return 0;
}
