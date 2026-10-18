/* Deliberately wrong version of func_8004F7E8: the last two statements are
 * swapped.  GCC 2.7.2 keeps the source order of the stores, so this assembles
 * to the same ten instructions in a different order -> MISMATCH, first
 * difference at offset 0x4 (the second lhu loads +0x42 instead of +0x44).
 * Used by test_match_fn.py to prove the oracle says no when it should. */
#include "ac1_types.h"

typedef struct {
    u8 pad[0x40];
    u16 x;          /* +0x40 */
    u16 y;          /* +0x42 */
    u16 w;          /* +0x44 */
} Func8004F7E8Obj;

void func_8004F7E8(Func8004F7E8Obj *o)
{
    o->x += 64;
    o->w += 16;
    o->y += 32;
}
