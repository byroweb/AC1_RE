#include "ac1_types.h"
/* ovl201 func_8008F08C - return the first word of entry `idx` of one of eleven
 * record tables, chosen by `kind` (switch 0..9, jump table at 0x8004CCF0); kind 9
 * covers two tables, entries 0..15 and 16.. . Other kinds return nothing. */

typedef struct {
    s32 value;
    u8 pad[40];
} Func8008F08CRec44;

typedef struct {
    s32 value;
    u8 pad[44];
} Func8008F08CRec48;

typedef struct {
    s32 value;
    u8 pad[48];
} Func8008F08CRec52;

typedef struct {
    s32 value;
    u8 pad[52];
} Func8008F08CRec56;

typedef struct {
    s32 value;
    u8 pad[56];
} Func8008F08CRec60;

typedef struct {
    s32 value;
    u8 pad[68];
} Func8008F08CRec72;

typedef struct {
    s32 value;
    u8 pad[92];
} Func8008F08CRec96;

extern Func8008F08CRec60 D_800B9218[];
extern Func8008F08CRec56 D_800B9470[];
extern Func8008F08CRec52 D_800B9E18[];
extern Func8008F08CRec72 D_800B9518[];
extern Func8008F08CRec48 D_800BAFF0[];
extern Func8008F08CRec96 D_800BACF0[];
extern Func8008F08CRec44 D_800BB140[];
extern Func8008F08CRec48 D_800BABD0[];
extern Func8008F08CRec52 D_800BA418[];
extern Func8008F08CRec44 D_800BA158[];
extern Func8008F08CRec52 D_800BAB00[];

s32 func_8008F08C(u8 kind, u8 idx)
{
    switch (kind) {
    case 0:
        return D_800B9218[idx].value;
    case 1:
        return D_800B9470[idx].value;
    case 2:
        return D_800B9E18[idx].value;
    case 3:
        return D_800B9518[idx].value;
    case 4:
        return D_800BAFF0[idx].value;
    case 5:
        return D_800BACF0[idx].value;
    case 6:
        return D_800BB140[idx].value;
    case 7:
        return D_800BABD0[idx].value;
    case 8:
        return D_800BA418[idx].value;
    case 9:
        if (idx >= 16) {
            return D_800BAB00[idx - 16].value;
        }
        return D_800BA158[idx].value;
    }
}
