#include "ac1_types.h"
typedef struct {
    s16 h0;         /* +0x00 */
    s8 b2;          /* +0x02 */
    u8 pad[3];
    u8 b6;          /* +0x06 */
} Func8004CF0CObj;
typedef struct {
    s16 v;          /* +0x00 */
    u8 pad[8];
} Func8004CF0CEntA;   /* stride 10 */
typedef struct {
    s16 v;          /* +0x00 */
    u8 pad[6];
} Func8004CF0CEntB;   /* stride 8 */
extern Func8004CF0CEntA D_80042086[];
extern Func8004CF0CEntB D_80041BD0[];
extern void func_80021940(s32);
void func_8004CF0C(Func8004CF0CObj *o)
{
    s16 unused[4];  /* never touched; retail frame is 32 bytes, i.e. 8 bytes of locals the code does not use */
    s16 v = D_80041BD0[D_80042086[o->b6].v].v;
    if (v != -1 && v != 254) {
        o->h0 = 0;
        func_80021940(o->b2);
    }
}
