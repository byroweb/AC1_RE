#include "ac1_types.h"
typedef struct {
    u8 pad[0x74];
    void (*handler)(void);  /* +0x74 */
} Func800551A0Obj;
extern void func_800551B0(void);
void func_800551A0(Func800551A0Obj *o)
{
    o->handler = func_800551B0;
}
