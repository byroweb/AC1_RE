#include "ac1/ac1.h"

#define ABS(x) ((x) >= 0 ? (x) : -(x))

typedef struct {
    void *def;          /* +0x00 */
    s32 dist;           /* +0x04 */
    u8 pad08[0x0E];
    u16 flags;          /* +0x16 */
    s16 dx;             /* +0x18 */
    s16 dz;             /* +0x1A */
} TgtScore;

s32 func_800743A0(void *a_, TgtScore *b, void *target)
{
    TgtScore *a = a_;
    s32 sa;
    s32 sb;

    if (b->def == 0) {
        return -(a->def != 0);
    }
    if (a->def == 0) {
        return 1;
    }
    sb = (ABS(b->dx) * 2 + ABS(b->dz) * 4 + (b->dist >> 6)) << 4;
    if (b->flags & 2) {
        sb >>= 2;
    }
    if (b->def == target) {
        sb >>= 1;
    }
    sa = (ABS(a->dx) * 2 + ABS(a->dz) * 4 + (a->dist >> 6)) << 4;
    if (a->flags & 2) {
        sa >>= 2;
    }
    if (a->def == target) {
        sa >>= 1;
    }
    return sa - sb;
}
