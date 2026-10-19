#include "ac1_types.h"
typedef struct {
    u32 pad;
    u32 a[8];       /* +0x04 */
    u32 b[8];       /* +0x24 */
    u32 c[8];       /* +0x44 */
} Func80070A00Tab;
u32 func_80070A00(Func80070A00Tab *t, u32 i, u32 j, u32 k)
{
    if (i < 8 && j < 8 && k < 8) {
        return t->a[i] & t->b[j] & t->c[k];
    }
    return 0;
}
