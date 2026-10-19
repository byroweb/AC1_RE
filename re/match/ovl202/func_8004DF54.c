#include "ac1_types.h"
typedef struct {
    u8 pad[8];
} Func8004DF54Item;
typedef struct {
    u8 pad[2];
    u8 b2;                      /* +0x02 */
    u8 b3;                      /* +0x03 */
    u8 pad2[0x20 - 4];
    Func8004DF54Item item[6];   /* +0x20 */
} Func8004DF54Obj;
extern void func_800131D0(Func8004DF54Item *, s32, s32);
void func_8004DF54(Func8004DF54Obj *o, s16 *p, s16 *q)
{
    o->b3 = o->b2;
    func_800131D0(&o->item[0], o->b3, p[0]);
    func_800131D0(&o->item[1], o->b3, p[1]);
    func_800131D0(&o->item[2], o->b3, p[2]);
    func_800131D0(&o->item[3], o->b3, q[0]);
    func_800131D0(&o->item[4], o->b3, q[1]);
    func_800131D0(&o->item[5], o->b3, q[2]);
}
