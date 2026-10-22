#include "ac1/ac1.h"

typedef struct {
    void (*fn)();       /* +0x00 */
    s32 pad4;
    s32 snd;            /* +0x08 */
} Func801C64ACTask;
typedef struct {
    u8 pad0[0x14];
    Func801C64ACTask *task; /* +0x14 */
    u8 pad18[0x10];
    u8 id;              /* +0x28 */
} Func801C64ACObj;
extern Func801C64ACTask *func_80073674(s32);
extern s32 func_80014D78(s32);
extern void func_801C5FD4();

void func_801C64AC(Func801C64ACObj *o)
{
    Func801C64ACTask *t;

    t = func_80073674(4);
    o->task = t;
    t->fn = func_801C5FD4;
    if (o->id == 0xFF) {
        o->task->snd = -1;
    } else {
        o->task->snd = func_80014D78(o->id);
    }
}
