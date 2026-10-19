/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0030): with the
 * fire_flags low nibble clear, call func_8004CE44 with the def's byte +0x3A at the
 * entity's position; otherwise func_8004CC30 with byte +0x3B and the buffer at
 * *D_801A5DC0 + 0x1000. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x3A];
    u8 id_a;            /* +0x3A */
    u8 id_b;            /* +0x3B */
} Func801C55B4Def;
typedef struct {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x108 - 0x10];
    Func801C55B4Def *def; /* +0x108 */
    u8 pad10c[0x148 - 0x10C];
    u16 fire_flags;     /* +0x148 */
} Func801C55B4Obj;
extern u8 *D_801A5DC0;
extern void func_8004CE44(s32 id, AcPos *pos, s32 a, s32 b);
extern void func_8004CC30(s32 id, u8 *buf, AcPos *pos, s32 a, s32 b, s32 c, s32 d);
void func_801C55B4(Func801C55B4Obj *o)
{
    if (!(o->fire_flags & 0xF))
        func_8004CE44(o->def->id_a, &o->pos, 100, -80);
    else
        func_8004CC30(o->def->id_b, D_801A5DC0 + 0x1000, &o->pos, 100, 8000, 10000, -48);
}
