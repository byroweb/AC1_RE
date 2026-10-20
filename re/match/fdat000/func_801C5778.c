/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0037): clear one
 * AI flag bit of the entity, the bit index coming from the record's byte +0x09. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x09];
    u8 bit;             /* +0x09 */
} Func801C5778Rec;
void func_801C5778(AcEntity *e, Func801C5778Rec *r)
{
    e->flags &= ~(1 << r->bit);
}
