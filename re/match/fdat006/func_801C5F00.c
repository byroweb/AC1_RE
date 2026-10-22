#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x2C];
    s32 busy;           /* +0x2C */
    u8 pad30[0x10];
} Func801C5F00Slot;     /* 0x40 */
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
} Func801C5F00Obj;
extern Func801C5F00Slot D_801D0B68[];

void func_801C5F00(Func801C5F00Obj *o)
{
    s32 n;
    Func801C5F00Slot *s;

    if (o->count != 0) {
        n = o->count;
        s = &D_801D0B68[o->first];
        do {
            if (s->busy != 0) {
                return;
            }
            s++;
        } while (--n != 0);
        o->state = -1;
        o->timer = 0;
        o->flags |= 0x8000;
    }
}
