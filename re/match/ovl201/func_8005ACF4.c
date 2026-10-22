#include "ac1/ac1.h"
typedef struct {
    u8 b[0xA0];
} Blk0xA0;
extern u8 D_80031A58[];     /* equipped part ids */
extern u8 D_800B70C4[];
extern u16 D_80039CA0;
extern u16 D_800B70D0;
extern Blk0xA0 D_80031A94;
extern Blk0xA0 D_80031B34;
extern Blk0xA0 D_800B70D4;
extern Blk0xA0 D_800B7174;
void func_8005ACF4(void)
{
    s16 i;
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_800B70C4[i];
    }
    D_80039CA0 = D_800B70D0;
    D_80031A94 = D_800B70D4;
    D_80031B34 = D_800B7174;
}
