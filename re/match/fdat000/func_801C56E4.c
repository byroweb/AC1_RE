/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0035): clear one
 * AI flag bit of the entity, the bit index coming from the record's byte +0x2D. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x2D];
    u8 bit;             /* +0x2D */
} Func801C56E4Rec;
void func_801C56E4(AcEntity *e, Func801C56E4Rec *r)
{
    e->flags &= ~(1 << r->bit);
}
