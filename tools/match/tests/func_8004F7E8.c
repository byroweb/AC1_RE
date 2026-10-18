/* ovl201 func_8004F7E8 - 10 instructions, leaf, no external symbols.
 *
 * Retail bytes (overlay 201 @ 0x8004F7E8, 40 bytes):
 *   lhu v0,64(a0); lhu v1,68(a0); addiu v0,v0,64; sh v0,64(a0); lhu v0,66(a0);
 *   addiu v1,v1,16; sh v1,68(a0); addiu v0,v0,32; jr ra; sh v0,66(a0)
 *
 * Three u16 fields of one struct get bumped.  Only the field types/offsets and
 * the statement order matter for the match; the field names are guesses.
 * This is the version that gives MATCH (see test_match_fn.py).
 */
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
    o->y += 32;
    o->w += 16;
}
