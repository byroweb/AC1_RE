/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0036): clear one
 * AI flag bit of the entity, the bit index coming from the record's byte +0x06. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x06];
    u8 bit;             /* +0x06 */
} Func801C573CRec;
void func_801C573C(AcEntity *e, Func801C573CRec *r)
{
    e->flags &= ~(1 << r->bit);
}
