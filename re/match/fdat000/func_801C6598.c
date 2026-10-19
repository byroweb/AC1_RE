/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0024): per-frame
 * driver keyed on the mover state at +0x6E: states 3..11 run the flag/colour pair
 * 801C55B4 + 801C565C (else 801C5634), states 0..20 run 801C56C8 (state 8 also
 * fires func_8004CE44), and the def's window +0x46..+0x47 picks between 801C62A8,
 * func_800745D0 at the lower edge, and switching the entity off past the upper one. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x2C];
    u8 b2c;             /* +0x2C */
    u8 b2d;             /* +0x2D */
    u8 pad2e[0x3C - 0x2E];
    u8 x3c[0x46 - 0x3C]; /* +0x3C */
    u8 lo;              /* +0x46 */
    u8 hi;              /* +0x47 */
    u8 pad48[2];
    u8 b4a;             /* +0x4A */
} Func801C6598Def;
typedef struct {
    u8 m64[2];
    u8 b66;             /* +0x66 */
    u8 pad67[0x6E - 0x67];
    s16 phase;          /* +0x6E */
} Func801C6598Mover;
typedef struct Func801C6598Obj Func801C6598Obj;
struct Func801C6598Obj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x36 - 0x10];
    u16 faction;        /* +0x36 */
    u8 pad38[0x41 - 0x38];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Func801C6598Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    Func801C6598Mover mover; /* +0x64 */
    u8 pad70[0x108 - 0x70];
    Func801C6598Def *def; /* +0x108 */
    u8 x10c[4];         /* +0x10C */
};
extern void func_80075FC0(Func801C6598Obj *o);
extern void func_801C55B4(Func801C6598Obj *o);
extern void func_801C565C(Func801C6598Obj *o, Func801C6598Def *d);
extern void func_801C5634(Func801C6598Obj *o, Func801C6598Def *d);
extern void func_8004CE44(s32 id, AcPos *pos, s32 a, s32 b);
extern void func_801C56C8(Func801C6598Obj *o, Func801C6598Def *d);
extern void func_801C56E4(Func801C6598Obj *o, Func801C6598Def *d);
extern void func_801C62A8(Func801C6598Obj *o, Func801C6598Def *d, void *p);
extern void func_800745D0(Func801C6598Obj *o, void *p, s32 a, s32 b, s32 c, s32 e, s32 f);
extern void func_8007A0E4(Func801C6598Obj *o);
extern s32 func_8004F080(void *mover, AcPos *pos);
void func_801C6598(Func801C6598Obj *o)
{
    Func801C6598Def *d = o->def;

    func_80075FC0(o);
    if (o->mover.phase >= 3 && o->mover.phase <= 11) {
        func_801C55B4(o);
        func_801C565C(o, d);
    } else {
        func_801C5634(o, d);
    }
    if (o->mover.phase >= 0 && o->mover.phase <= 20) {
        if (o->mover.phase == 8)
            func_8004CE44(d->b4a, &o->pos, 120, 0);
        func_801C56C8(o, d);
    } else {
        func_801C56E4(o, d);
    }
    if (o->mover.phase > d->lo && o->mover.phase < d->hi) {
        func_801C62A8(o, d, d->x3c);
    } else if (o->mover.phase == d->lo) {
        func_800745D0(o, o->x10c, d->b2c, d->b2d, 2, 0, 1);
    } else if (!(o->mover.phase < d->hi)) {
        if (o->state) {
            o->mover.b66 = 1;
            o->faction &= 0xFFEF;
            func_8007A0E4(o);
        }
    }
    if (!func_8004F080(&o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
