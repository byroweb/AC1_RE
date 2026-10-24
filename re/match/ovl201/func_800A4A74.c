/* ovl201 0x800A4A74: matched (--defsym D_8017F428_x=0x8017F428); the else-branch index k = D_8017F424 * 8 in its own block local drops the unused 8-byte stack slot */
#include "ac1/ac1.h"
/* ovl201 0x800A4A74 - movie player per-frame upload: swap the two decode
 * buffers, advance the destination slice by its width, switch to the other
 * screen half when it passes the end, and LoadImage the decoded slice. */
typedef struct {
    s16 x;
    s16 y;
    s16 len;
    s16 pad;
} MovSeg;

typedef struct {
    u32 *buf[2];        /* 0x8017F408 */
    volatile s32 cur;   /* 0x8017F410 */
} MovBufs;

extern MovBufs D_8017F408;
extern MovSeg D_8017F414[2];
extern s32 D_8017F424;
extern RECT D_8017F428;
extern s16 D_8017F428_x;    /* same address, read as a scalar */
extern s16 D_8017F42A;
extern s16 D_8017F42C;
extern s16 D_8017F42E;
extern s32 D_8017F430;
extern s32 D_800417CC;
extern void StCdInterrupt(void);
extern void func_800B506C(u32 *buf, s32 size, s32 half, volatile s32 *cur);
extern int LoadImage(RECT *r, u32 *p);

void func_800A4A74(void)
{
    RECT r;
    s32 prev;
    volatile s32 *pc;

    if (D_800417CC != 0) {
        StCdInterrupt();
        D_800417CC = 0;
    }
    r = D_8017F428;
    pc = &D_8017F408.cur;
    prev = *pc;
    *pc = *pc == 0;
    D_8017F428_x += D_8017F42C;
    if (D_8017F428_x < D_8017F414[D_8017F424].x + D_8017F414[D_8017F424].len) {
        func_800B506C(D_8017F408.buf[D_8017F408.cur], D_8017F42C * D_8017F42E / 2, D_8017F424, pc);
    } else {
        D_8017F430 = 1;
        D_8017F424 = D_8017F424 == 0;
        {
            s32 k = D_8017F424 * 8;
            D_8017F428_x = *(s16 *)((u8 *)&D_8017F414[0].x + k);
            D_8017F42A = *(s16 *)((u8 *)&D_8017F414[0].y + k);
        }
    }
    LoadImage(&r, D_8017F408.buf[prev]);
}
