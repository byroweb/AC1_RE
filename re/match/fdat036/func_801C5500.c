#include "ac1/ac1.h"

typedef struct {
    u8 pad0[8];
    u8 pad8[0x2E];
    u16 f36;
    u8 pad38[4];
    u8 mode;            /* +0x3C */
    u8 pad3d[4];
    u8 state;           /* +0x41 */
    u8 pad42[0xE];
    void (*fn)(void *, u8);     /* +0x50 */
    u8 pad54[0x10];
    u8 pad64[0x68];
    u16 vx;             /* +0xCC */
    s16 pad_ce;
    u16 vz;             /* +0xD0 */
    s16 cnt;            /* +0xD2 */
} Ent;
extern s32 SquareRoot0();
extern void func_800143B8();
extern void func_80014728();
extern s32 func_8004F080();
extern void func_80075FC0();

void func_801C5500(Ent *e)
{
    s16 v[3];
    s32 a;
    s32 sq;
    s16 t;
    s16 t2;

    func_80075FC0();
    switch (e->mode) {
    case 1:
        func_80014728(*(s16 *)((u8 *)e + 0x12), v);
    upd:
        e->vx += (v[0] * 5) >> 6;
        e->vz += (v[2] * 5) >> 6;
        t = *(s16 *)&e->vx;
        t2 = *(s16 *)&e->vz;
        sq = SquareRoot0(t * t + t2 * t2);
        if (sq > 3200) {
            e->vx = (*(s16 *)&e->vx * 3200) / sq;
            e->vz = (*(s16 *)&e->vz * 3200) / sq;
        }
        break;
    case 2:
        func_80014728(*(s16 *)((u8 *)e + 0x12) + 0x200, v);
        goto upd;
    case 3:
        func_80014728(*(s16 *)((u8 *)e + 0x12) - 0x200, v);
        goto upd;
    case 6:
        func_80014728(*(s16 *)((u8 *)e + 0x12) + 0x800, v);
        goto upd;
    case 4:
        func_80014728(*(s16 *)((u8 *)e + 0x12) + 0x600, v);
        goto upd;
    case 5:
        func_80014728(*(s16 *)((u8 *)e + 0x12) + 0xA00, v);
        goto upd;
    case 7:
        if (e->cnt == 0) {
            e->f36 &= 0xFFEF;
        } else {
            e->cnt = e->cnt - 1;
        }
        goto upd;
    default:
        func_800143B8((u8 *)e + 0xCC, 0x140, 0xC80, (u8 *)e + 0xCC);
        break;
    }
    if (func_8004F080((u8 *)e + 0x64, (u8 *)e + 8) == 0) {
        if (e->fn != 0) {
            e->fn(e, e->state);
        }
    }
}
