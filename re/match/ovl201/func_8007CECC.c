#include "ac1_types.h"
typedef struct {
    u8 pad[0x74];
    void (*update)(void);   /* +0x74 */
} Func8007CECCObj;
extern u8 D_8005C2D0[];
extern u8 D_801BC4F8[];
extern s16 D_801A2318;
extern void *func_80050078(void *key);
extern void func_8005BA2C(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_800210F8(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_8007CF64(void);
void func_8007CECC(Func8007CECCObj *o)
{
    if (func_80050078(D_8005C2D0) == (void *)D_801BC4F8) {
        func_8005BA2C(0x20, 0x28, 0x88, 0x80, 3, 0, 0);
        func_800210F8(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        o->update = func_8007CF64;
    }
}
