/* fdat004 (66 twins): give an object a script record (func_80073674(0)) at +0x14 whose
 * handler is func_801C6720. */
#include "ac1/ac1.h"
typedef struct {
    void (*fn)();       /* +0x00 */
} Func801C6734Rec;
typedef struct {
    u8 pad0[0x14];
    Func801C6734Rec *rec; /* +0x14 */
} Func801C6734Obj;
extern Func801C6734Rec *func_80073674(s32 kind);
extern void func_801C6720();
void func_801C6734(Func801C6734Obj *o)
{
    Func801C6734Rec *r;

    r = func_80073674(0);
    o->rec = r;
    r->fn = func_801C6720;
}
