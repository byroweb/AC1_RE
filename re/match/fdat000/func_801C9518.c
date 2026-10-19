/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0028): reset an
 * entity's per-life state: scale +0x164 by 4, store func_80073904's result in three
 * timers, clear the indices at +0x124/+0x126 to -1 and the rest of the block, then run
 * fn_move(e, 0) and fn_state(e, state) when they are set. */
#include "ac1/ac1.h"
typedef struct Func801C9518Obj Func801C9518Obj;
struct Func801C9518Obj {
    u8 pad0[0x41];
    u8 state;           /* +0x41 */
    u8 b42;             /* +0x42 */
    u8 pad43;
    s16 h44;            /* +0x44 */
    u8 pad46[0x50 - 0x46];
    void (*fn_state)(Func801C9518Obj *, u8);  /* +0x50 */
    void (*fn_move)(Func801C9518Obj *, s32);  /* +0x54 */
    u8 pad58[0x124 - 0x58];
    s16 h124;           /* +0x124 */
    s16 h126;           /* +0x126 */
    s16 h128;           /* +0x128 */
    s16 h12a;           /* +0x12A */
    s16 h12c;           /* +0x12C */
    s16 h12e;           /* +0x12E */
    u8 b130;            /* +0x130 */
    u8 b131;            /* +0x131 */
    u8 b132;            /* +0x132 */
    u8 b133;            /* +0x133 */
    u8 b134;            /* +0x134 */
    u8 pad135;
    u8 b136;            /* +0x136 */
    u8 pad137[0x164 - 0x137];
    u16 h164;           /* +0x164 */
};
extern s32 func_80073904();
void func_801C9518(Func801C9518Obj *o)
{
    s32 t;

    o->b42 = 0;
    o->b130 = 0;
    o->h164 <<= 2;
    t = func_80073904();
    o->h12a = t;
    o->h12c = t;
    o->h44 = t;
    o->h126 = -1;
    o->h124 = -1;
    o->h128 = 0;
    o->b136 = 0;
    o->h12e = 0;
    o->b133 = 0;
    o->b134 = 0;
    o->b132 = 0;
    o->b131 = 0;
    if (o->fn_move)
        o->fn_move(o, 0);
    if (o->fn_state)
        o->fn_state(o, o->state);
}
