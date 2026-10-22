#include "ac1/ac1.h"

typedef struct {                /* 0x18 */
    u8 used;                    /* +0x00 0xFF = free, else the pixel mode */
    u8 b;                       /* +0x01 */
    u8 c;                       /* +0x02 */
    u8 d;                       /* +0x03 */
    RECT rect;                  /* +0x04 */
    s16 f0c;                    /* +0x0C */
    u8 pad0e[2];
    u32 *buf;                   /* +0x10 */
    u32 *buf2;                  /* +0x14 */
} VramSave;

extern VramSave *func_80056C30(void);
extern u32 *func_80016190(s32);

void func_80056C6C(s32 x, s32 y, s32 b, s32 d, s32 mode, s32 w, s32 h) {
    VramSave *v;

    /* retail reads mode/w/h from their stack slots at every use: they are
     * addressable in the source (taking the address emits no code) */
    { s32 *dummy = &mode; dummy = &w; dummy = &h; }
    v = func_80056C30();
    if (v != 0) {
        v->used = mode;
        v->f0c = 0;
        v->b = b;
        v->d = d;
        v->c = 0;
        v->rect.x = x;
        v->rect.y = y;
        switch (mode) {
        case 1:
            v->rect.w = w >> 2;
            v->rect.h = h;
            v->buf = func_80016190(v->rect.w * (s16)h * 4);
            break;
        case 2:
            v->rect.w = w;
            v->rect.h = h;
            v->buf = func_80016190(v->rect.w * (s16)h * 4);
            break;
        case 3:
            v->rect.w = w >> 1;
            v->rect.h = h;
            v->buf = func_80016190(v->rect.w * (s16)h * 4);
            v->buf2 = func_80016190(v->rect.w * v->rect.h * 4);
            break;
        }
        StoreImage(&v->rect, v->buf);
        DrawSync(0);
    }
}
