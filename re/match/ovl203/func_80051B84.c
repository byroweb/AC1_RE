#include "ac1/ac1.h"

typedef struct Pt16 {
    s16 x, y;
} Pt16;

typedef struct Pilot {
    u8 unk00[0x10];
    u16 flags;                  /* +0x10 */
} Pilot;

extern Pilot *D_80039D30;
extern u8 D_8004ADCC[];
extern u8 D_8004ADE0[];
extern u8 D_8004ADF4[];
extern u8 D_800896D0[];
extern u8 D_800896D4[];
extern Pt16 D_800896E0;
extern Pt16 D_800896E4;
extern u8 D_800896E8[];
extern Pt16 D_800896F4;
extern u8 D_800896F8[];
extern u8 D_80089704[];
extern Pt16 D_80089710;
extern u8 D_80089714[];
extern Pt16 D_80089720;
extern u8 D_80089724[];
extern Pt16 D_80089730;
extern u8 D_80089734[];
extern Pt16 D_80089740;
extern u8 D_80089758[];
extern u8 D_8008975C[];
extern u32 D_8019B48C;
extern u16 D_8019B492;
extern void func_80051E1C(s32 x, s32 y, s32 w, s32 h, s32 n, u8 *s);
extern void func_8005DFF8(s32 x, s32 y, s32 w, s32 h, u8 *col, s32 a, s32 b);
extern void func_80060374(Pt16 *p, u8 *a, s32 b, u8 *c, s32 d, s32 e);

void func_80051B84(void)
{
    u32 *f = &D_8019B48C;
    if (*f & 0x300) {
        func_8005DFF8(0, 0, 319, 239, D_800896D0, 10, 2);
        switch (*f & 0x300) {
        case 0x100:
            func_80051E1C(124, 110, 10, 56, 4, D_8004ADCC);
            break;
        case 0x200:
            func_80051E1C(126, 120, 10, 56, 8, D_8004ADE0);
            break;
        case 0x300:
            func_80051E1C(128, 130, 10, 56, 15, D_8004ADF4);
            break;
        }
    }
    if (!(D_8019B48C & 0x40)) {
        switch (D_80039D30->flags & 0x30) {
        case 0x30:
            func_80060374(&D_800896E0, D_800896D4, 0, D_80089758, 9, 2);
            break;
        case 0x10:
            func_80060374(&D_800896E4, D_800896E8, 0, D_80089758, 9, 2);
            break;
        case 0x20:
            func_80060374(&D_800896F4, D_800896F8, 0, D_8008975C, 9, 2);
            break;
        }
    }
    if (D_8019B48C & 0x1000) {
        if (D_8019B492 < 23) {
            func_80060374(&D_80089710, D_80089704, 0, D_80089758, 9, 2);
        } else {
            func_80060374(&D_80089720, D_80089714, 0, D_80089758, 9, 2);
        }
    } else if (D_8019B48C & 0x40) {
        func_80060374(&D_80089730, D_80089724, 0, D_80089758, 9, 2);
    } else if (D_8019B48C & 0x80) {
        func_80060374(&D_80089740, D_80089734, 0, D_80089758, 9, 2);
    }
}
