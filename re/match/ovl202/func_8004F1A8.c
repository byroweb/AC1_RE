#include "ac1/ac1.h"
/* ovl202 0x8004F1A8 - build the AC model n: write the level digits into the
 * label string, then load each fitted part (0xFF = none) into the model
 * buffer D_8019F520 (func_800537FC, slot n + 0x80, sub-slot n * 16 + 0xE0..).
 * Back weapon (+0x22) is skipped for arm kinds 8..15. */

extern u8 D_8004121B_a[]; /* level, then the fitted part ids (0xFF none) */
#define D_8004121B D_8004121B_a[0x0]
#define D_8004121D D_8004121B_a[0x2]
#define D_8004121E D_8004121B_a[0x3]
#define D_8004121F D_8004121B_a[0x4]
#define D_80041220 D_8004121B_a[0x5]
#define D_80041221 D_8004121B_a[0x6]
#define D_80041222 D_8004121B_a[0x7]
#define D_80041223 D_8004121B_a[0x8]
#define D_80041224 D_8004121B_a[0x9]
extern s32 D_8004A394;
extern char D_8008D928[];
extern struct { s32 buf; } D_8019F520_s; /* model load cursor; struct for the la base form */
#define D_8019F520 D_8019F520_s.buf
typedef struct { u8 pad[0x2C]; } Rec2C;
extern Rec2C D_8019F538[];
extern u8 *D_801A5F34[];
extern void func_80016678(s32, char *);
extern void func_800165E4(s32, s32, s32);
extern s32 func_8005360C(s32, s32, s32, s32);
extern s32 func_800537FC(s32, s32, s32, s32, s32);
extern void func_80073AD8(Rec2C *, s32, s32);

void func_8004F1A8(s32 n)
{
    s32 save;
    s32 slot;
    s32 k;
    u8 *p;

    D_8008D928[1] = D_8004121B / 20 + '0';
    D_8008D928[5] = D_8004121B / 10 + '0';
    save = D_8004A394;
    D_8008D928[6] = D_8004121B % 10 + '0';
    func_80016678(0, D_8008D928);
    func_800165E4(0, 1, D_8019F520);
    func_80073AD8(&D_8019F538[n], D_8019F520, n);
    slot = n + 0x80;
    k = n * 16;
    D_8019F520 += func_8005360C(D_8019F520, 0, 0, slot);
    if (D_8004121D != 0xFF) {
        D_8019F520 += func_800537FC(D_8019F520, 0, D_8004121D + 2, slot, k + 0xE0);
    }
    if (D_8004121E != 0xFF) {
        D_8019F520 += func_800537FC(D_8019F520, 0, D_8004121E + 10, slot, k + 0xE1);
    }
    if (D_8004121F != 0xFF) {
        D_8019F520 += func_800537FC(D_8019F520, 0, D_8004121F + 44, slot, k + 0xE2);
    }
    if (D_80041220 != 0xFF) {
        D_8019F520 += func_800537FC(D_8019F520, 0, D_80041220 + 88, slot, k + 0xE3);
    }
    if (D_80041221 != 0xFF) {
        D_8019F520 += func_800537FC(D_8019F520, 0, D_80041221 + 60, slot, k + 0xE4);
    }
    if ((u32)(D_80041220 - 8) >= 8 && D_80041222 != 0xFF) {
        D_8019F520 += func_800537FC(D_8019F520, 0, D_80041222 + 70, slot, k + 0xE5);
    }
    if (D_80041223 != 0xFF) {
        D_8019F520 += func_800537FC(D_8019F520, 0, D_80041223 + 108, slot, k + 0xE6);
    }
    if (D_80041224 != 0xFF) {
        D_8019F520 += func_800537FC(D_8019F520, 0, D_80041224 + 142, slot, k + 0xE7);
    }
    p = D_801A5F34[slot];
    p += *(s32 *)(p + 8);
    *(s32 *)(p + 4) |= 1;
    D_8004A394 = save;
}
