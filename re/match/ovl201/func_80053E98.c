#include "ac1/ac1.h"

/* one save entry of the card directory list, 0x2C each at 0x801F5098 */
typedef struct {
    char name[0x24];    /* +0x00 */
    s16 a;              /* +0x24 */
    u8 pad26[2];
    s32 b;              /* +0x28 */
} SaveEntry;
extern SaveEntry D_801F5098[];

/* text rows on screen, 27 bytes each at 0x8017E474 */
typedef struct {
    char no[9];
    char a[9];
    char b[9];
} SaveRowText;
extern SaveRowText D_8017E474[];

extern u8 D_801A257C;
extern u8 D_800B6F28[];
extern u8 D_800B6F34[];
extern u8 D_8004B0F0[];
extern void func_8005D754(s32 a, void *p, s32 x, s32 y, s32 n);
extern void func_8005D80C(s32 a, void *s, s32 b, s32 c, s32 x, s32 y);
extern UiObj *func_8005CB94(void *s, s32 x, s32 y, s32 n);
extern void func_800518D4(char *buf, s16 n);
extern void func_800519A4(char *buf, s32 n);
extern void func_80051A74(char *buf, s32 n);

void func_80053E98(UiObj *o)
{
    UiWork *w = &o->w;
    u8 rows;
    u8 n;
    s16 i;
    s16 e;
    s16 e2;
    u8 top;
    s16 y;

    rows = D_801A257C;
    if (o->w.raw[0x2B] != 0) {
        rows++;
    }
    if (rows > 5) {
        rows = 5;
    }
    for (i = 0; i < rows; i++) {
        y = i * 0x18;
        func_8005D754(3, D_800B6F28, 0, y, 8);
        func_8005D754(3, D_800B6F34, 0xF8, y, 8);
    }
    top = w->raw[0x2A];
    n = D_801A257C - top;
    if (n > 5) {
        n = 5;
    }
    for (i = 0, e = top; i < n; i++) {
        func_800518D4(D_8017E474[i].no, e + 1);
        y = i * 0x18 + 4;
        func_8005D80C(2, D_8017E474[i].no, 0x10, 8, y, 8);
        func_8005CB94(D_801F5098[e].name, 0x20, y, 8)->unk14 = 2;
        func_800519A4(D_8017E474[i].a, D_801F5098[e].a);
        func_8005D80C(2, D_8017E474[i].a, 0x10, 0xB0, y, 8);
        func_80051A74(D_8017E474[i].b, D_801F5098[e].b);
        e++;
        func_8005D80C(2, D_8017E474[i].b, 0x10, 0xD8, y, 8);
    }
    if (w->raw[0x2B] != 0 && rows > n) {
        func_8005CB94(D_8004B0F0, 8, n * 0x18 | 4, 8)->unk14 = 2;
    }
}
