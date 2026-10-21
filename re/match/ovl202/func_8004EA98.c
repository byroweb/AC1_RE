#include "ac1/ac1.h"
/* ovl202 0x8004EA98 - set a (kind, value) pair; when the kind changes, also
 * store extra at +0x0E.  Unchanged pairs are left alone. */

typedef struct {
    u8 kind;            /* +0x00 */
    u8 unk01[0xB];
    s16 value;          /* +0x0C */
    s16 extra;          /* +0x0E */
} KindVal;

void func_8004EA98(KindVal *p, s32 kind, s32 value, s32 extra)
{
    if (p->kind == kind) goto check;
    p->extra = extra;
store:
    p->kind = kind;
    p->value = value;
    return;
check:
    if (p->value != value) goto store;
}
