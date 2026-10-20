/* fdat008 (15 twins): set state 2 and the update handler at 0x801CBEAC, then run the
 * shared init at 0x801CBE60 (same shape as fdat016 func_801C6AE0). */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x36];
    u16 state;          /* +0x36 */
    u8 pad2[0x108 - 0x38];
    void *handler;      /* +0x108 */
} Func801C5418Obj;
extern void func_801CBEAC();
extern void func_801CBE60(Func801C5418Obj *o);
void func_801C5418(Func801C5418Obj *o)
{
    o->state = 2;
    o->handler = func_801CBEAC;
    func_801CBE60(o);
}
