#include "ac1_types.h"
typedef struct {
    u8 pad[0x74];
    void (*handler)(void);  /* +0x74 */
} Func8009A7C4Obj;
extern void func_8009A7D4(void);
void func_8009A7C4(Func8009A7C4Obj *o)
{
    o->handler = func_8009A7D4;
}
