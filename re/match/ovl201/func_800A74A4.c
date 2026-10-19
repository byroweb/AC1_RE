#include "ac1_types.h"
typedef struct {
    u8 pad[0x74];
    void (*fn)(void);   /* +0x74 */
} Func800A74A4Obj;
extern void func_800A74B4(void);
void func_800A74A4(Func800A74A4Obj *o)
{
    o->fn = func_800A74B4;
}
