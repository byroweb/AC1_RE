#include "ac1_types.h"
typedef struct {
    u32 a[16];      /* +0x00 */
    u32 b[16];      /* +0x40 */
} Func800B4F48Obj;
extern u32 D_800BE214[];    /* header word, then the 16-word copy of a at +4 */
extern u32 D_800BE258[];
extern u8 D_800BE298[];
extern void func_800B5248(void *, s32);
/* The parameter register itself is walked as the source pointer (retail has no copy
 * of a0).  Retail saves the object pointer first, but cse (the "(set REG0 REG1)"
 * rewrite in cse_insn) swaps that copy with the parameter copy whenever they are
 * adjacent insns and o outlives src, which costs an extra move; a second,
 * unused parameter leaves a dead copy between them, which is the only
 * arrangement found that reproduces retail's instruction order. */
Func800B4F48Obj *func_800B4F48(u32 *src, s32 unused)
{
    Func800B4F48Obj *o;
    u32 *d;
    s32 i;

    o = (Func800B4F48Obj *)src;
    d = &D_800BE214[1];
    for (i = 15; i != -1; i--) {
        *d++ = *src++;
    }
    d = D_800BE258;
    src = o->b;
    for (i = 15; i != -1; i--) {
        *d++ = *src++;
    }
    func_800B5248(D_800BE214, 32);
    func_800B5248(D_800BE298, 32);
    return o;
}
