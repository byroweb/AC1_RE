#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    s16 state;          /* +0x06 */
    u8 pad8[0x2C];
} Func801C5F64Ent;      /* 0x34 */
typedef struct {
    u8 pad0[6];
    s16 state;          /* +0x06 */
    u8 pad8[0x0C];
    s32 timer;          /* +0x14 */
    u8 pad18[4];
    u16 flags;          /* +0x1C */
    u8 pad1E[5];
    u8 count;           /* +0x23 */
    u16 first;          /* +0x24 */
} Func801C5F64Obj;
extern Func801C5F64Ent D_80198B08[];

void func_801C5F64(Func801C5F64Obj *o)
{
    s32 n;
    Func801C5F64Ent *e;

    if (o->count != 0) {
        n = o->count;
        e = &D_80198B08[o->first];
        do {
            if (e->state != -1) {
                return;
            }
            e++;
        } while (--n != 0);
        o->state = -1;
        o->timer = 0;
        o->flags |= 0x8000;
    }
}
