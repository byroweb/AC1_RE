#include "ac1_types.h"
/* ovl201 func_8004F588 - switch on the s16 at +0x7C (cases 1..5, jump table at
 * 0x8004AE74), each case calling func_8004F624 with its own (x, y, z) triple; any
 * other value does nothing. The five calls are cross-jumped into one jal. */

typedef struct {
    u8 pad[0x7C];
    s16 kind;   /* +0x7C */
} Func8004F588Obj;

extern void func_8004F624(Func8004F588Obj *obj, s32 x, s32 y, s32 z);

void func_8004F588(Func8004F588Obj *obj)
{
    switch (obj->kind) {
    case 1:
        func_8004F624(obj, -512, 512, -3036);
        break;
    case 2:
        func_8004F624(obj, -512, 512, -2012);
        break;
    case 3:
        func_8004F624(obj, 0, 512, -2012);
        break;
    case 4:
        func_8004F624(obj, 512, 512, -2012);
        break;
    case 5:
        func_8004F624(obj, 512, 512, -3036);
        break;
    }
}
