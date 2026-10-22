#include "ac1/ac1.h"
/* ovl202 0x8008B320: re-aim an entity's mover (+0x64) from its position (+0x10) */
int func_8004E22C(void *, void *, void *, SVECTOR *, SVECTOR *);
void func_8004DEAC(void *, int, SVECTOR *, SVECTOR *);
typedef struct {
    u8 pad0[4];
    u8 *ent;
} Obj8008B320;
void func_8008B320(Obj8008B320 *o)
{
    SVECTOR a;
    SVECTOR b;
    u8 *e = o->ent;
    func_8004DEAC(e + 0x64, func_8004E22C(e + 0x64, e + 0x10, e + 0x10, &a, &b), &a, &b);
}
