#include "ac1/ac1.h"

typedef struct {
    s8 unk0;            /* +0x00 */
    s8 mode;            /* +0x01 */
    s8 kind;            /* +0x02 */
    u8 pad03[0x57];
    u16 top;            /* +0x5A */
    u8 pad5c[0x3A];
    u16 start[1];       /* +0x96 */
} ShopState;
extern ShopState D_8019C090;
extern u8 D_8019C1AC[];
extern u8 D_8019C24C[];
extern void func_80090E88(s8 kind, s16 id);
extern void func_80093AE8(UiObj *o);

void func_80093A20(UiObj *o)
{
    s32 kind;
    s16 id;
    s16 i;
    s16 unused[4];       /* 8 frame bytes retail reserves but never touches */

    if (D_8019C090.mode == 0) {
        kind = D_8019C090.kind;
        i = D_8019C090.top + D_8019C090.start[kind];
        id = (s8)D_8019C24C[i];
    } else {
        id = (s8)D_8019C24C[(s16)D_8019C090.top];
        kind = D_8019C1AC[(s16)D_8019C090.top];
    }
    func_80090E88(kind, id);
    DrawSync(0);
    o->update = func_80093AE8;
}
