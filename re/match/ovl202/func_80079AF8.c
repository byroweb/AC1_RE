#include "ac1_types.h"
/* ovl202 0x80079AF8  size 188: decay two velocities toward zero */
typedef struct {
    u8 pad[0xCC];
    s16 vx;             /* +0xCC */
    s16 padCE;
    s16 vz;             /* +0xD0 */
} Func80079AF8Obj;
extern u8 D_80041201;

static inline void func_80079AF8_decay(s16 *p, u8 *f)
{
    s16 v;

    v = *p;
    if (v > 0) {
        if (v > *f) {
            *p = v - *f;
        } else {
            *p = 0;
        }
    } else {
        if (v < -*f) {
            *p = v + *f;
        } else {
            *p = 0;
        }
    }
}

void func_80079AF8(Func80079AF8Obj *o)
{
    s32 unused[2];

    func_80079AF8_decay(&o->vx, &D_80041201);
    func_80079AF8_decay(&o->vz, &D_80041201);
}
