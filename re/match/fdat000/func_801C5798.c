/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0029): unless the
 * byte at +0x3D is 0xB0, run func_8004DE88 on the mover and the vector at +0xCC and
 * scale that vector by 128; then clear the AI flags, start func_800775B8 with the
 * think routine func_801C5CB8, and set +0x3C and state to 0x41. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x3C];
    u8 mode;            /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e[0x41 - 0x3E];
    u8 state;           /* +0x41 */
    u8 pad42[0x64 - 0x42];
    u8 mover[0xCC - 0x64]; /* +0x64 AcMover */
    u16 vec[3];         /* +0xCC */
    u8 padd2[0x15C - 0xD2];
    u32 flags;          /* +0x15C */
} Func801C5798Obj;
extern void func_8004DE88(void *mover, u16 *vec);
extern void func_800775B8(Func801C5798Obj *o, void *arg, s32 a, s32 b, u16 *vec, void *fn);
extern void func_801C5CB8();
void func_801C5798(Func801C5798Obj *o, void *arg)
{
    if (o->kind != 0xB0) {
        func_8004DE88(o->mover, o->vec);
        o->vec[0] <<= 7;
        o->vec[1] = 0;
        o->vec[2] <<= 7;
    }
    o->flags = 0;
    func_800775B8(o, arg, 1, 0x200, o->vec, func_801C5CB8);
    o->mode = 0x41;
    o->state = 0x41;
}
