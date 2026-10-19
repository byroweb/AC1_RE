#include "ac1_types.h"
typedef struct {
    u8 pad[2];
    u8 b2;                  /* +0x02 */
    u8 b3;                  /* +0x03 */
    u8 pad2;
    u8 b5;                  /* +0x05 */
    u8 pad3[0x6C - 6];
    s32 w6C;                /* +0x6C */
    void (*init)(void);     /* +0x70 */
    void (*update)(void);   /* +0x74 */
    void (*draw)(void);     /* +0x78 */
} Func8008DD50Obj;
extern Func8008DD50Obj *func_800501BC(s32);
extern void func_8008DDA0(void);
extern void func_800507F4(void);
void func_8008DD50(void)
{
    Func8008DD50Obj *o = func_800501BC(4);
    o->init = (void (*)(void))func_8008DD50;
    o->update = func_8008DDA0;
    o->draw = func_800507F4;
    o->b2 = 0;
    o->b3 = 0;
    o->w6C = 0;
    o->b5 = 0;
}
