#include "ac1/ac1.h"

typedef struct {
    void (*fn)();       /* +0x00 */
} Func801C5FDCTask;
typedef struct {
    u8 pad0[0x14];
    Func801C5FDCTask *task; /* +0x14 */
    u8 pad18[0x0A];
    u8 kind;            /* +0x22 */
} Func801C5FDCObj;
extern Func801C5FDCTask *func_80073674(s32);
extern void func_801C5F00();
extern void func_801C5F64();

void func_801C5FDC(Func801C5FDCObj *o)
{
    Func801C5FDCTask *t;

    t = func_80073674(0);
    o->task = t;
    switch (o->kind) {
    case 0:
        t->fn = func_801C5F00;
        break;
    case 1:
        t->fn = func_801C5F64;
        break;
    }
}
