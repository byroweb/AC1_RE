/* ovl202 func_8007A108 - set one AI flag bit of an entity, the bit index coming
 * from the record's byte +0x03. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[3];
    u8 bit;         /* +0x03 */
} Func8007A108Src;
void func_8007A108(AcEntity *e, Func8007A108Src *s)
{
    e->flags |= 1 << s->bit;
}
