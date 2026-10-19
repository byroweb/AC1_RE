#include "ac1_types.h"
/* ovl201 func_800A4340 - allocates a kind-2 front-end object, installs its
 * handlers and resets its fields; the text pointer and the +0x48 field depend
 * on whether the argument is 'L'.  Both arms of the last if end with the same
 * two stores (the compiler cross-jumps them after scheduling, which is why
 * retail has the stores before the return move). */
typedef struct {
    u8 pad[5];
    u8 f5;          /* +0x05 */
    u8 pad6[0x0A - 6];
    s16 fa;         /* +0x0A */
    u8 *text;       /* +0x0C */
    u8 pad10[4];
    s16 f14;        /* +0x14 */
    s16 f16;        /* +0x16 */
    u8 pad18[0x40 - 0x18];
    s16 f40;        /* +0x40 */
    s16 f42;        /* +0x42 */
    s16 f44;        /* +0x44 */
    s32 f48;        /* +0x48 */
    s32 f4c;        /* +0x4C */
    s32 f50;        /* +0x50 */
    u8 pad54[0x70 - 0x54];
    void *init;     /* +0x70 */
    void (*update)(void);   /* +0x74 */
    void (*draw)(void);     /* +0x78 */
} Func800A4340Obj;
extern u8 D_800BDBD8[];
extern u8 D_800BDBE4[];
extern Func800A4340Obj *func_800501BC(s32 kind);
extern void func_800507FC(void);
extern void func_800507F4(void);
Func800A4340Obj *func_800A4340(u8 kind)
{
    Func800A4340Obj *o;
    o = func_800501BC(2);
    o->init = func_800A4340;
    o->update = func_800507FC;
    o->draw = func_800507F4;
    o->fa = 3;
    o->f16 = 4;
    o->f14 = 3;
    if (kind == 'L') {
        o->text = D_800BDBD8;
    } else {
        o->text = D_800BDBE4;
    }
    o->f5 = 0;
    o->f40 = 0;
    o->f42 = 0;
    o->f44 = 0;
    if (kind == 'L') {
        o->f48 = 0;
        o->f4c = 0;
        o->f50 = 0;
    } else {
        o->f48 = 256;
        o->f4c = 0;
        o->f50 = 0;
    }
    return o;
}
