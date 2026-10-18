#include "ac1_types.h"
typedef struct {
    u8 pad[0x7E];
    u16 a;          /* +0x7E */
    u16 b;          /* +0x80 */
} Func8006DCECSub;
typedef struct {
    u8 pad[0x98];
    Func8006DCECSub *sub;   /* +0x98 */
} Func8006DCECObj;
extern u32 D_801A2568;
extern void func_8006C324(void);
void func_8006DCEC(Func8006DCECObj *o)
{
    Func8006DCECSub *s = o->sub;
    if (D_801A2568 & 0x40) {
        if (s->a >= 172 && s->a < 300 && s->b >= 36 && s->b < 164) {
            func_8006C324();
        }
    }
}
