#include "ac1/ac1.h"

typedef struct {
    u8 a;               /* +0x00 */
    u8 b;               /* +0x01 */
    u16 c;              /* +0x02 */
    u8 d;               /* +0x04 */
    u8 e;               /* +0x05 */
    u8 once;            /* +0x06 */
} Func801C5C6CDef;
typedef struct {
    u8 pad0[0x2C];
    s32 timer;          /* +0x2C */
    u8 pad30[8];
    Func801C5C6CDef *def; /* +0x38 */
} Func801C5C6CObj;
extern void func_8008A0B0(s32, s32, s32, s32, s32);

s32 func_801C5C6C(Func801C5C6CObj *o, s32 msg, s32 **arg, ...)
{
    Func801C5C6CDef *d;

    if (msg == 0x8100) {
        if (((*arg)[3] & 7) != 2) {
            return 0;
        }
        d = o->def;
        func_8008A0B0(d->a, d->b, d->c, d->d, d->e);
        if (o->def->once == 1) {
            o->timer = 0;
        }
        return 1;
    }
    return 0;
}
