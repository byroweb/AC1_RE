/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0021): react to an
 * event st: 0..8 and 48..55 go to 801C7264 or a random/straight-up push; other events
 * (unless the mover is in mode 22) push the entity by its own or its target's height
 * via 801C72C4, except that kind 192 with type 4 and a high roll only records st. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x0A];
    u16 height;         /* +0x0A */
} Func801C737CTarget;
typedef struct {
    u8 pad0[0x2C];
    u16 height;         /* +0x2C */
    u8 pad2e[0x3D - 0x2E];
    u8 kind;            /* +0x3D */
    u8 pad3e[0x41 - 0x3E];
    u8 state;           /* +0x41 */
    u8 pad42[0x48 - 0x42];
    Func801C737CTarget *target; /* +0x48 */
    u8 pad4c[0x64 - 0x4C];
    u8 mode;            /* +0x64 AcMover.mode */
    u8 pad65[0xEC - 0x65];
    u8 type;            /* +0xEC */
    u8 paded[0xFC - 0xED];
    u16 spread;         /* +0xFC */
    u8 padfe;
    u8 use_own;         /* +0xFF */
} Func801C737CObj;
extern s32 func_80014D78(s32 n);
extern s32 func_8002A4A0(void);
extern void func_801C7264(Func801C737CObj *o, s32 st);
extern void func_801C72C4(Func801C737CObj *o, s32 st, AcPos *v);
extern void func_801C7320(Func801C737CObj *o, s32 st, AcPos *v);
void func_801C737C(Func801C737CObj *o, s32 st)
{
    AcPos v;

    if (st >= 0 && st < 9) {
        if (o->kind == 176) {
            func_801C7264(o, 56);
            return;
        }
        if (o->kind == 192) {
            v.z = 0;
            v.x = 0;
            v.y = 32767;
            func_801C72C4(o, 65, &v);
            return;
        }
        v.x = func_80014D78(500) - 250;
        v.z = func_80014D78(500) - 250;
        v.y = 0;
        func_801C7320(o, st, &v);
    } else if (st >= 48 && st < 56) {
        if (o->kind == 192) {
            v.z = 0;
            v.x = 0;
            v.y = 32767;
            func_801C72C4(o, 65, &v);
            return;
        }
        func_801C7264(o, st);
    } else if (o->mode != 22) {
        if (o->kind == 192) {
            if (func_8002A4A0() < 9831 || o->type != 4) {
                v.z = 0;
                v.x = 0;
                if (o->target == 0 || o->use_own == 1)
                    v.y = o->height - func_80014D78(o->spread) - 100;
                else
                    v.y = o->target->height - func_80014D78(o->spread) - 100;
                func_801C72C4(o, st, &v);
            } else {
                o->state = st;
            }
        } else {
            v.z = 0;
            v.x = 0;
            if (o->target == 0 || o->use_own == 1)
                v.y = o->height - func_80014D78(o->spread) - 100;
            else
                v.y = o->target->height - func_80014D78(o->spread) - 100;
            func_801C72C4(o, 64, &v);
        }
    }
}
