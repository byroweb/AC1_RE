/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0025): when the
 * yaw differs from func_80075FC0's angle, call func_8004CC30 (id 13, buffer
 * *D_801A5DC0 + 0x1300) at the position; then damp the X/Z velocity by 1/8, snapping
 * |v| <= 512 to 0, clear Y, and run fn_state unless the mover step reports done. */
#include "ac1/ac1.h"
typedef struct Func801C6A28Obj Func801C6A28Obj;
struct Func801C6A28Obj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x41 - 0x14];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Func801C6A28Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 AcMover */
    u16 vel[3];         /* +0xCC */
};
extern u8 *D_801A5DC0;
extern s32 func_80075FC0(Func801C6A28Obj *o);
extern void func_8004CC30(s32 id, u8 *buf, AcPos *pos, s32 a, s32 b, s32 c, s32 d);
extern s32 func_8004F080(void *mover, AcPos *pos);
void func_801C6A28(Func801C6A28Obj *o)
{
    s32 v;
    s32 t;

    t = func_80075FC0(o) & 0xFFF;
    if (o->yaw != t)
        func_8004CC30(13, D_801A5DC0 + 0x1300, &o->pos, 40, 8000, 10000, 0);
    o->vel[1] = 0;
    v = (s16)o->vel[0];
    if ((v < 0 ? -v : v) <= 512)
        o->vel[0] = 0;
    else
        o->vel[0] -= v >> 3;
    v = (s16)o->vel[2];
    if ((v < 0 ? -v : v) <= 512)
        o->vel[2] = 0;
    else
        o->vel[2] -= v >> 3;
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
