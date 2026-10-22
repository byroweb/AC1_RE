/* ovl203 func_80077E10: versus link-cable pad exchange, once per frame.
 * Sends this side's pad word in an 8-byte 'R'..'C' packet (random byte and a checksum
 * of the frame counter and both players' pads), reads the peer's packet, flags a
 * checksum / random-byte mismatch (D_8019B48C.flags bits 0x100/0x200, 0x300 on a link
 * failure, with an 80-frame timer), then latches both players' pads. */
#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    u8 hdr;             /* +0 'R' */
    u8 sum;             /* +1 checksum */
    u16 pad;            /* +2 pad word */
    u8 unk4[2];
    u8 rnd;             /* +6 random byte */
    u8 tail;            /* +7 'C' */
} LinkPkt;

typedef struct {
    s32 pad_raw;        /* +0 */
    s32 pad_raw_prev;   /* +4 */
    s32 pad_cur;        /* +8 */
    s32 pad_prev;       /* +C */
} PadBlk;

extern PadBlk *D_80039D30;
extern PadBlk *D_80039D34;
typedef struct {
    s32 rfd;            /* +0x00 */
    s32 wfd;            /* +0x04 */
    volatile LinkPkt rx;       /* +0x08 */
    u8 unk10[4];
    volatile LinkPkt tx;       /* +0x14 */
} LinkIo;
extern LinkIo D_8004A284;
extern volatile s32 D_800895C4;
extern u8 D_800895C8;
extern u16 D_8008A21C;
typedef struct {
    s32 flags;
    u16 timer;
} LinkErr;
extern LinkErr D_8019B48C;

extern s32 rand(void);
extern u16 func_80016A84(s32 a);
extern s32 func_8004BEB0(void);
extern s32 func_8001628C(void);
extern s32 read(s32 fd, void *buf, s32 n);
extern s32 write(s32 fd, void *buf, s32 n);

void func_80077E10(void)
{
    u8 r;
    u16 pad;
    s32 a;
    s32 b;

    if (D_800895C8 != 0)
        goto clear;
    r = rand();
    pad = func_80016A84(1);
    if (func_8004BEB0() == 0)
        goto fail;
    if (D_8004A284.rx.sum != D_8004A284.tx.sum) {
        D_8019B48C.flags |= 0x100;
        if (D_8019B48C.timer == 0)
            D_8019B48C.timer = 80;
    }
    if (D_8004A284.rx.rnd != D_8004A284.tx.rnd) {
        D_8019B48C.flags |= 0x200;
        if (D_8019B48C.timer == 0)
            D_8019B48C.timer = 80;
    }
    D_80039D30->pad_raw_prev = D_80039D30->pad_raw;
    D_80039D30->pad_raw = D_8004A284.tx.pad;
    D_80039D34->pad_raw_prev = D_80039D34->pad_raw;
    D_80039D34->pad_raw = D_8004A284.rx.pad;
    D_8004A284.tx.pad = pad;
    D_8004A284.tx.hdr = 'R';
    D_8004A284.tx.tail = 'C';
    D_8004A284.tx.rnd = r;
    a = D_80039D30->pad_cur;
    b = D_80039D34->pad_cur;
    D_8004A284.tx.sum = D_8008A21C + a + (a >> 8) + b + (b >> 8);
    D_8004A284.rx.hdr = D_8004A284.rx.tail = 0;
    read(D_8004A284.rfd, &D_8004A284.rx, 8);
    D_800895C4 = func_8001628C();
    write(D_8004A284.wfd, &D_8004A284.tx, 8);
    if (D_800895C8 == 0)
        goto latch;
fail:
    D_8019B48C.flags |= 0x300;
    if (D_8019B48C.timer == 0)
        D_8019B48C.timer = 80;
clear:
    D_80039D34->pad_cur = 0;
    D_80039D30->pad_cur = 0;
latch:
    D_80039D30->pad_prev = D_80039D30->pad_cur;
    D_80039D34->pad_prev = D_80039D34->pad_cur;
    if (!(D_8019B48C.flags & 0x400)) {
        D_80039D30->pad_cur = D_80039D30->pad_raw;
        D_80039D34->pad_cur = D_80039D34->pad_raw;
    } else {
        D_80039D34->pad_cur = 0;
        D_80039D30->pad_cur = 0;
    }
    D_8008A21C++;
}
