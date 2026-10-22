#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    s16 kind;           /* +0x06 */
    u8 pad8[0x14];
    u16 flags;          /* +0x1C */
    u8 pad1e[0x16];
} Slot;                 /* 0x34 */
typedef struct {
    u8 pad0[0x0E];
    s16 id;             /* +0x0E */
    u8 pad10[8];
} Kind;                 /* 0x18 */
extern Slot D_80198B08[];
extern Kind D_80198800[];

void func_801C6F10(s32 unused, s32 *score)
{
    s32 i;
    Slot *s;

    s = D_80198B08;
    i = 0;
    do {
        if (s->kind != -1 && !(s->flags & 0x8000) && D_80198800[s->kind].id == 13) {
            if (i == 59 || i == 61) {
                *score -= 500;
            } else {
                *score -= 200;
            }
        }
        i++;
        s++;
    } while (i < 512);
}
