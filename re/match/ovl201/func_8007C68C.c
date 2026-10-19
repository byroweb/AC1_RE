#include "ac1_types.h"
/* ovl201 func_8007C68C - allocates a front-end object, installs its three
 * handlers (init = this function), clears a few fields, runs the first update
 * and returns the object (the "move v0,s0" before the last store is the return). */
typedef struct {
    u8 pad[2];
    u8 f2;          /* +0x02 */
    u8 f3;          /* +0x03 */
    u8 pad4;
    u8 f5;          /* +0x05 */
    u8 pad6[0x6C - 6];
    s32 f6c;        /* +0x6C */
    void *init;     /* +0x70 */
    void (*update)(void);   /* +0x74 */
    void (*draw)(void);     /* +0x78 */
    u8 pad7c[4];
    u8 f80;         /* +0x80 */
} Func8007C68CObj;
extern Func8007C68CObj *func_800501BC(s32 kind);
extern void func_8007C6FC(Func8007C68CObj *o);
extern void func_8007C7D8(void);
extern void func_800507F4(void);
Func8007C68CObj *func_8007C68C(void)
{
    Func8007C68CObj *o;
    o = func_800501BC(0);
    o->init = func_8007C68C;
    o->update = func_8007C7D8;
    o->draw = func_800507F4;
    o->f2 = 0;
    o->f3 = 0;
    o->f6c = 0;
    o->f5 = 0;
    func_8007C6FC(o);
    o->f80 = 0;
    return o;
}
