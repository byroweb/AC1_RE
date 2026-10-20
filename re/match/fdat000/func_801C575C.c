/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0041): toggle one
 * AI flag bit of the entity, the bit index coming from the record's byte +0x09. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x09];
    u8 bit;             /* +0x09 */
} Func801C575CRec;
void func_801C575C(AcEntity *e, Func801C575CRec *r)
{
    e->flags ^= 1 << r->bit;
}
