#include "ac1_types.h"
typedef struct {
    u8 pad[0x74];
    void (*fn)(void);   /* +0x74 */
} Func8008BF90Obj;
extern void func_8008BFA0(void);
void func_8008BF90(Func8008BF90Obj *o)
{
    o->fn = func_8008BFA0;
}
