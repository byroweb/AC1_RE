#include "ac1/ac1.h"

typedef struct {
    u32 *vlcbuf[2];     /* +0x00 */
    s32 vlcid;          /* +0x08 */
    u16 *imgbuf[2];     /* +0x0C */
    s32 imgid;          /* +0x14 */
    RECT rect[2];       /* +0x18 */
    s32 rectid;         /* +0x28 */
    RECT slice;         /* +0x2C */
    s32 isdone;         /* +0x34 */
} DECENV;

typedef struct {
    u16 id;
    u16 type;
    u16 secCount;
    u16 nSectors;
    u32 frameCount;     /* +0x08 */
    u32 frameSize;
    u16 width;          /* +0x10 */
    u16 height;         /* +0x12 */
} StHEADER;

extern int StGetNext(u32 **addr, u32 **header);
extern int ClearImage(RECT *rect, u8 r, u8 g, u8 b);
extern s16 D_8017F44C;
extern s32 D_8017F448;
extern s32 D_800BDCBC;
extern s32 D_800BDCC0;

u32 *func_800A4C70(DECENV *dec)
{
    u32 *addr;
    StHEADER *sector;
    s32 cnt;

    cnt = 0x800000;
    while (StGetNext(&addr, (u32 **)&sector)) {
        if (--cnt == 0) {
            return 0;
        }
    }
    if (sector->frameCount >= D_8017F44C) {
        D_8017F448 = 1;
    }
    if (D_800BDCBC != sector->width || D_800BDCC0 != sector->height) {
        RECT rect;

        rect.x = 0;
        rect.y = 0;
        rect.w = 480;
        rect.h = 480;
        ClearImage(&rect, 0, 0, 0);
        D_800BDCBC = sector->width;
        D_800BDCC0 = sector->height;
    }
    dec->rect[0].w = dec->rect[1].w = D_800BDCBC * 3 / 2;
    dec->rect[0].h = dec->rect[1].h = D_800BDCC0;
    dec->slice.h = D_800BDCC0;
    return addr;
}
