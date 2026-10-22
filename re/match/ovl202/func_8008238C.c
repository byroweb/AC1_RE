#include "ac1/ac1.h"
/* ovl202 0x8008238C: player AC input -> movement direction state (+0x41) message handler.
 * msg 4 in mode 1 forces dir 0x60; msg 0 maps the held pad bits through the key map
 * (0x80041230..) to 1..10 (walking) or 0x41..0x4B (kind 192, airborne). */
typedef struct {
    u32 held;           /* 0x80040FA8 */
    u8 pad04[0x0A - 0x04];
    s16 b0a;            /* 0x80040FB2 */
    u8 pad0c[0x18 - 0x0C];
    u16 flags;          /* 0x80040FC0 */
    u8 pad1a[0x272 - 0x1A];
    u8 mode;            /* 0x8004121A */
    u8 pad273[0x288 - 0x273];
    u16 key_fwd;        /* 0x80041230 */
    u16 key_back;       /* 0x80041232 */
    u16 key_a;          /* 0x80041234 */
    u16 key_b;          /* 0x80041236 */
    u16 pad238;
    u16 key_c;          /* 0x8004123A */
    u8 pad23c[0x298 - 0x294];
    u16 key_left;       /* 0x80041240 */
    u16 pad29a;
    u16 key_right;      /* 0x80041244 */
} PadCtl;
extern PadCtl D_80040FA8;

int func_8008238C(AcEntity *o, int msg, ...)
{
    u32 m;

    if (msg == 4 && D_80040FA8.mode == 1) {
        ((u8 *)o)[0x3C] = 0x60;
    }
    if (msg != 0)
        return 0;
    m = D_80040FA8.held & D_80040FA8.key_c;
    if (m == 0) {
        D_80040FA8.flags &= ~2;
    }
    if (o->mover.mode == 22)
        return 0;
    if (((u8 *)o)[0x3D] == 192) {
        if ((D_80040FA8.held & (D_80040FA8.key_fwd | D_80040FA8.key_left)) == (D_80040FA8.key_fwd | D_80040FA8.key_left)) o->state = 0x42;
        else if ((D_80040FA8.held & (D_80040FA8.key_fwd | D_80040FA8.key_right)) == (D_80040FA8.key_fwd | D_80040FA8.key_right)) o->state = 0x43;
        else if ((D_80040FA8.held & (D_80040FA8.key_back | D_80040FA8.key_left)) == (D_80040FA8.key_back | D_80040FA8.key_left)) o->state = 0x45;
        else if ((D_80040FA8.held & (D_80040FA8.key_back | D_80040FA8.key_right)) == (D_80040FA8.key_back | D_80040FA8.key_right)) o->state = 0x46;
        else if (D_80040FA8.key_fwd & D_80040FA8.held) o->state = 0x41;
        else if (D_80040FA8.key_back & D_80040FA8.held) o->state = 0x44;
        else if (D_80040FA8.key_left & D_80040FA8.held) o->state = 0x47;
        else if (D_80040FA8.key_right & D_80040FA8.held) o->state = 0x48;
        else if (D_80040FA8.held & D_80040FA8.key_a) o->state = 0x49;
        else if (D_80040FA8.held & D_80040FA8.key_b) o->state = 0x4A;
        else if (D_80040FA8.held & D_80040FA8.key_c) o->state = 0x40;
        else o->state = 0x4B;
    } else if (((u8 *)o)[0x3C] != 0x60 && !(D_80040FA8.flags & 2) && !((D_80040FA8.flags & 1) && D_80040FA8.mode == 1) && m != 0) {
        o->state = 0x40;
    } else {
            if ((D_80040FA8.held & (D_80040FA8.key_fwd | D_80040FA8.key_left)) == (D_80040FA8.key_fwd | D_80040FA8.key_left)) o->state = 2;
        else if ((D_80040FA8.held & (D_80040FA8.key_fwd | D_80040FA8.key_right)) == (D_80040FA8.key_fwd | D_80040FA8.key_right)) o->state = 3;
        else if ((D_80040FA8.held & (D_80040FA8.key_back | D_80040FA8.key_left)) == (D_80040FA8.key_back | D_80040FA8.key_left)) o->state = 7;
        else if ((D_80040FA8.held & (D_80040FA8.key_back | D_80040FA8.key_right)) == (D_80040FA8.key_back | D_80040FA8.key_right)) o->state = 8;
        else if (D_80040FA8.key_fwd & D_80040FA8.held) o->state = 1;
        else if (D_80040FA8.key_back & D_80040FA8.held) o->state = 6;
        else if (D_80040FA8.key_left & D_80040FA8.held) o->state = 4;
        else if (D_80040FA8.key_right & D_80040FA8.held) o->state = 5;
        else if (D_80040FA8.held & D_80040FA8.key_a) o->state = 9;
        else if (D_80040FA8.held & D_80040FA8.key_b) o->state = 10;
        else {
            D_80040FA8.b0a = 0;
            o->state = 0;
            return 0;
        }
    }
    return 0;
}
