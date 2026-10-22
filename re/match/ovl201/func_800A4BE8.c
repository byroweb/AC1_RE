#include "ac1/ac1.h"

typedef struct {
    s32 buf[2];         /* +0x00 */
    s32 cur;            /* +0x08 */
} StrmHead;

extern void *func_800A4C70(StrmHead *s);
extern void func_800B5618(void *r, s32 buf);
extern void func_8001A458(void *r);

s32 func_800A4BE8(StrmHead *s)
{
    void *r;
    s32 i;

    for (i = 0x800000; i != 0; i--) {
        r = func_800A4C70(s);
        if (r != 0) {
            s->cur = s->cur == 0;
            func_800B5618(r, s->buf[s->cur]);
            func_8001A458(r);
            return 1;
        }
    }
    return -1;
}
