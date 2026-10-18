/* ovl201 func_800505F0 - switch (jump table at 0x8004AEEC, 19 entries, only the
 * even cases 2..20 have bodies) selecting one of ten 0xD0-byte item arrays, then a
 * loop that stores the second argument into every item whose first byte is set.
 * The loop also runs for an out-of-range kind with p/end uninitialised, exactly
 * like the retail code (no default case). */
#include "ac1_types.h"

typedef struct {
    u8 used;        /* +0x00 */
    u8 pad[0x15];
    s16 value;      /* +0x16 */
    u8 pad2[0xD0 - 0x18];
} Func800505F0Item;

extern Func800505F0Item D_801A36F8[];
extern Func800505F0Item D_801A48D8[];
extern Func800505F0Item D_801A7318[];
extern Func800505F0Item D_801A9D58[];
extern Func800505F0Item D_801AD7D8[];
extern Func800505F0Item D_801B1258[];
extern Func800505F0Item D_801B4CD8[];
extern Func800505F0Item D_801B7718[];
extern Func800505F0Item D_801B9118[];
extern Func800505F0Item D_801BAB18[];

void func_800505F0(s16 kind, s16 value)
{
    Func800505F0Item *p;
    Func800505F0Item *end;

    switch (kind) {
    case 2:
        p = D_801A36F8;
        end = p + 19;
        break;
    case 4:
        p = D_801A48D8;
        end = p + 49;
        break;
    case 6:
        p = D_801A7318;
        end = p + 49;
        break;
    case 8:
        p = D_801A9D58;
        end = p + 69;
        break;
    case 10:
        p = D_801AD7D8;
        end = p + 69;
        break;
    case 12:
        p = D_801B1258;
        end = p + 69;
        break;
    case 14:
        p = D_801B4CD8;
        end = p + 49;
        break;
    case 16:
        p = D_801B7718;
        end = p + 29;
        break;
    case 18:
        p = D_801B9118;
        end = p + 29;
        break;
    case 20:
        p = D_801BAB18;
        end = p + 29;
        break;
    }
    for (; p <= end; p++) {
        if (p->used) {
            p->value = value;
        }
    }
}
