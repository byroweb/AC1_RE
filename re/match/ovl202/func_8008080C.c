#include "ac1/ac1.h"
/* ovl202 0x8008080C: player AC input -> movement state (+0x41) message handler, boosting
 * variant of 0x8008238C: kind 0xB0 (boost) uses the boost timer 0x80040FB2, 0xC0 the
 * airborne set 0x41..0x4B, else the walking set 1..10. */
typedef struct {
    u32 held;           /* 0x80040FA8 */
    u8 pad04[0x0A - 0x04];
    s16 b0a;            /* 0x80040FB2 */
    u8 pad0c[0x18 - 0x0C];
    u16 flags;          /* 0x80040FC0 */
    u8 pad1a[0x288 - 0x1A];
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
} PadCtl80;
extern PadCtl80 D_80040FA8;
extern s16 D_80040FB2[];
extern u16 D_800411AA;

#define P D_80040FA8
#define BOTH(a, b) ((P.held & (P.a | P.b)) == (P.a | P.b))

int func_8008080C(AcEntity *o, int msg, ...)
{
    u32 m;
    u8 d;
    s16 *bp;

    if (msg != 0)
        return 0;
    m = P.held & P.key_c;
    bp = &P.b0a;
    if (m == 0) {
        P.flags &= ~2;
        bp = &P.b0a;
    }
    if (o->mover.mode == 22)
        return 0;
    if (((u8 *)o)[0x3D] == 0xB0) {
        if (*bp >= 0) {
            if (m != 0 && !(P.flags & 1)) {
                if (BOTH(key_fwd, key_left)) {
                    d = 49;
                set:
                    o->state = d;
                    D_80040FB2[0] = 5;
                    goto out;
                }
                if (BOTH(key_fwd, key_right)) {
                    d = 50;
                    goto set;
                }
                if (BOTH(key_back, key_left)) {
                    d = 54;
                    goto set;
                }
                if (BOTH(key_back, key_right)) {
                    d = 55;
                    goto set;
                }
                if (P.key_fwd & P.held) {
                    d = 48;
                    goto set;
                }
                if (P.key_back & P.held) {
                    d = 53;
                    goto set;
                }
                if (P.key_left & P.held) {
                    d = 51;
                    goto set;
                }
                if (P.key_right & P.held) {
                    d = 52;
                    goto set;
                }
                if (P.b0a == 0) {
                    P.flags |= 2;
                    o->state = 56;
                    return 0;
                }
                P.b0a--;
                return 0;
            }
            D_80040FB2[0] = -6;
            return 0;
        }
        if (m != 0 && !(P.flags & 1)) {
            *(s16 *)((u8 *)o + 0xCE) = -D_800411AA / 4;
            o->state = 64;
            P.b0a = 0;
            return 0;
        }
        if (D_80040FB2[0] == -1) {
            o->state = 56;
        } else {
            D_80040FB2[0]++;
        }
    } else if (((u8 *)o)[0x3D] == 0xC0) {
        if (BOTH(key_fwd, key_left)) o->state = 0x42;
        else if (BOTH(key_fwd, key_right)) o->state = 0x43;
        else if (BOTH(key_back, key_left)) o->state = 0x45;
        else if (BOTH(key_back, key_right)) o->state = 0x46;
        else if (P.key_fwd & P.held) o->state = 0x41;
        else if (P.key_back & P.held) o->state = 0x44;
        else if (P.key_left & P.held) o->state = 0x47;
        else if (P.key_right & P.held) o->state = 0x48;
        else if (P.held & P.key_a) o->state = 0x49;
        else if (P.held & P.key_b) o->state = 0x4A;
        else if (P.held & P.key_c) {
        c40:
            o->state = 0x40;
        } else o->state = 0x4B;
    } else {
        if (BOTH(key_fwd, key_left)) o->state = 2;
        else if (BOTH(key_fwd, key_right)) o->state = 3;
        else if (BOTH(key_back, key_left)) o->state = 7;
        else if (BOTH(key_back, key_right)) o->state = 8;
        else if (P.key_fwd & P.held) o->state = 1;
        else if (P.key_back & P.held) o->state = 6;
        else if (P.key_left & P.held) o->state = 4;
        else if (P.key_right & P.held) o->state = 5;
        else if (P.held & P.key_a) o->state = 9;
        else if (P.held & P.key_b) o->state = 10;
        else if (((u8 *)o)[0x3C] != 0x60 && !(P.flags & 2) && (P.held & P.key_c)) goto c40;
        else {
            D_80040FB2[0] = 0;
            o->state = 0;
            return 0;
        }
    }
out:
    return 0;
}
