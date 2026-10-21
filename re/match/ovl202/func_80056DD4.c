#include "ac1/ac1.h"

typedef struct {                /* 0x18 */
    u8 mode;                    /* +0x00 0xFF = free; 1/2/3 scroll kind */
    u8 period;                  /* +0x01 frames between steps */
    u8 timer;                   /* +0x02 */
    s8 speed;                   /* +0x03 */
    RECT rect;                  /* +0x04 */
    s16 ofs;                    /* +0x0C */
    u8 pad0e[2];
    u32 *buf;                   /* +0x10 */
    u32 *buf2;                  /* +0x14 */
} VramSave;

extern VramSave D_801AC8A4[];
extern s32 func_8002701C(s32);

void func_80056DD4(void) {
    VramSave *e;
    RECT r;
    u32 *p;
    u16 *src;
    u16 *dst;
    u8 *s;
    u8 *d;
    s32 i;
    s32 ang;
    s32 bytes;
    s32 rows;
    s32 k;

    e = D_801AC8A4;
    i = 8;
    while (--i != -1) {
        if (e->timer == 0) {
            e->timer = e->period;
            switch (e->mode) {
            case 1:
                e->ofs += e->speed;
                if (e->ofs >= e->rect.h) {
                    e->ofs -= e->rect.h;
                }
                r.x = e->rect.x;
                r.y = e->rect.y + e->ofs;
                r.w = e->rect.w;
                r.h = e->rect.h - e->ofs;
                LoadImage(&r, e->buf);
                if (e->ofs != 0) {
                    p = e->buf + ((r.w * r.h) >> 1);
                    r.y = e->rect.y;
                    r.h = e->ofs;
                    LoadImage(&r, p);
                }
                break;
            case 2:
                r.x = e->rect.x + e->ofs;
                r.y = e->rect.y;
                r.w = e->rect.w - e->ofs;
                r.h = e->rect.h;
                LoadImage(&r, e->buf);
                if (e->ofs != 0) {
                    p = e->buf + (r.w >> 1);
                    r.x = e->rect.x;
                    r.w = e->ofs;
                    LoadImage(&r, p);
                }
                if (e->speed > 0) {
                    if (e->ofs >= e->rect.w) {
                        e->speed = -e->speed;
                    }
                } else if (e->ofs == 0) {
                    e->speed = -e->speed;
                }
                e->ofs += e->speed;
                break;
            case 3:
                e->ofs += e->speed;
                ang = e->ofs;
                r.y = e->rect.y;
                r.x = e->rect.x;
                r.w = e->rect.w;
                r.h = 1;
                bytes = r.w * 2;
                src = (u16 *)e->buf;
                dst = (u16 *)e->buf2;
                rows = e->rect.h;
                do {
                    s = (u8 *)src + (func_8002701C(ang) >> 10) + 4;
                    d = (u8 *)dst;
                    k = bytes;
                    do {
                        *d++ = *s++;
                    } while (--k);
                    ang += 32;
                    rows--;
                    src += e->rect.w;
                    dst += e->rect.w;
                    r.y++;
                } while (rows != 0);
                LoadImage(&e->rect, e->buf2);
                break;
            }
        } else {
            e->timer--;
        }
        e++;
    }
}
