#include "ac1/ac1.h"
/* ovl201 0x8008EED4 - address of entry `idx` of the part table for shop
 * category `kind` (the name at +0; func_8008F08C reads the word at +0x24 of
 * the same records). Kind 9 covers two tables, entries 0..15 and 16.. . */

typedef struct { char name[0x24]; s32 price; u8 pad[60 - 0x28]; } PartRec60;
typedef struct { char name[0x24]; s32 price; u8 pad[56 - 0x28]; } PartRec56;
typedef struct { char name[0x24]; s32 price; u8 pad[52 - 0x28]; } PartRec52;
typedef struct { char name[0x24]; s32 price; u8 pad[72 - 0x28]; } PartRec72;
typedef struct { char name[0x24]; s32 price; u8 pad[48 - 0x28]; } PartRec48;
typedef struct { char name[0x24]; s32 price; u8 pad[96 - 0x28]; } PartRec96;
typedef struct { char name[0x24]; s32 price; u8 pad[44 - 0x28]; } PartRec44;

extern PartRec60 D_800B91F4[];
extern PartRec56 D_800B944C[];
extern PartRec52 D_800B9DF4[];
extern PartRec72 D_800B94F4[];
extern PartRec48 D_800BAFCC[];
extern PartRec96 D_800BACCC[];
extern PartRec44 D_800BB11C[];
extern PartRec48 D_800BABAC[];
extern PartRec52 D_800BA3F4[];
extern PartRec44 D_800BA134[];
extern PartRec52 D_800BAADC[];

void *func_8008EED4(u8 kind, u8 idx)
{
    switch (kind) {
    case 0:
        return &D_800B91F4[idx];
    case 1:
        return &D_800B944C[idx];
    case 2:
        return &D_800B9DF4[idx];
    case 3:
        return &D_800B94F4[idx];
    case 4:
        return &D_800BAFCC[idx];
    case 5:
        return &D_800BACCC[idx];
    case 6:
        return &D_800BB11C[idx];
    case 7:
        return &D_800BABAC[idx];
    case 8:
        return &D_800BA3F4[idx];
    case 9:
        if (idx >= 16) {
            return &D_800BAADC[idx - 16];
        }
        return &D_800BA134[idx];
    }
}
