#include "ac1/ac1.h"
/* ovl201 0x80075898: assembly screen part-spec panel. Pad 0x10 toggles the help
   window (obj id 9); while w->bAE is set it redraws the description lines of the
   current category and the spec values of the selected part (w->hA6, 255 = none). */

typedef struct AsmWork {
    void *src;          /* +0x00 (+0x7C) */
    UiObj *owner;       /* +0x04 */
    u8 b84, b85;        /* +0x08 */
    u8 b86, b87;        /* +0x0A */
    UiObj *sub;         /* +0x0C */
    u8 pad8c[4];
    u16 h90;            /* +0x14 part category */
    u16 h92;            /* +0x16 */
    s16 h94;            /* +0x18 */
    u8 b96;             /* +0x1A */
    u8 b97;
    u8 b98, b99, b9a;
    u8 pad9b;
    UiObj *bar;         /* +0x20 */
    u8 bA0, bA1;        /* +0x24 */
    s16 hA2, hA4;       /* +0x26 */
    u16 hA6;            /* +0x2A part index, 255 = none */
    u16 hA8;            /* +0x2C */
    u8 padAA;
    u8 bAB;             /* +0x2F */
    u16 hAC;            /* +0x30 */
    u8 bAE;             /* +0x32 spec panel shown */
    u8 padAF;
    u16 hB0;            /* +0x34 */
} AsmWork;

typedef struct AsmObj {
    u8 head[0x7C];
    AsmWork w;
} AsmObj;

/* part tables: two 18-byte names, then the price and the stats */
typedef struct Part4 {          /* 60 bytes */
    char name[18];
    char maker[18];
    s32 price;                  /* +0x24 */
    u16 h28, h2A, h2C, h2E, h30;
    u8 b32, b33, b34, b35, b36;
    u8 pad37;
    u16 h38;
    u8 b3A, b3B;
} Part4;

typedef struct Part0 {          /* 56 bytes */
    char name[18];
    char maker[18];
    s32 price;
    u16 h28, h2A, h2C, h2E, h30, h32;
    u8 b34, b35, b36;
    u8 pad37[1];
} Part0;

typedef struct Part1 {          /* 72 bytes */
    char name[18];
    char maker[18];
    s32 price;
    u16 h28, h2A, h2C, h2E, h30, h32;
    u16 h34, h36;
    u16 h38, h3A, h3C, h3E;
    u16 h40;
    u8 b42;
    u8 pad43[5];
} Part1;

typedef struct Part2 {          /* 52 bytes */
    char name[18];
    char maker[18];
    s32 price;
    u16 h28, h2A, h2C, h2E, h30;
    u16 h32;
} Part2;

typedef struct Part7 {          /* 44 bytes */
    char name[18];
    char maker[18];
    s32 price;
    u16 h28, h2A;
} Part7;

typedef struct Part6 {          /* 52 bytes */
    char name[18];
    char maker[18];
    s32 price;
    u16 h28, h2A;
    u8 b2C, b2D;
    u16 h2E;
    u8 b30, b31;
    u16 h32;
} Part6;

typedef struct Part3 {          /* 52 bytes */
    char name[18];
    char maker[18];
    s32 price;
    u16 h28, h2A, h2C, h2E, h30;
    u16 h32;
} Part3;

typedef struct Part5 {          /* 48 bytes */
    char name[18];
    char maker[18];
    s32 price;
    u16 h28, h2A, h2C, h2E;
} Part5;

typedef struct Part8 {          /* 96 bytes */
    char name[18];
    char maker[18];
    s32 price;
    u16 h28, h2A;
    u8 b2C, b2D;
    u8 pad2E[0x32];
} Part8;

typedef struct Wpn {            /* 24 bytes */
    u8 pad0[6];
    u8 b06;
    u8 pad7;
    u16 h08;
    u8 padA[2];
    u16 h0C;
    s16 h0E;
    u8 b10, b11, b12;
    u8 pad13;
    u16 h14;
    u8 pad16[2];
} Wpn;

extern u32 D_801A2568;
extern u32 D_801A256C;
extern char D_8017EE80[][10];
extern char **D_800BBA58[];
extern char *D_800BBA34[];
extern char *D_800BBA84[];
extern char *D_800BBA8C[];
extern char *D_800BBA9C;
extern char *D_800BBAA0;
extern char *D_800BBAA4[];
extern char *D_800BBAB8[];
extern char *D_800BBACC[];
extern char *D_800BBAD8[];
extern Part4 D_800B91F4[];
extern Part0 D_800B944C[];
extern Part1 D_800B94F4[];
extern Part2 D_800B9DF4[];
extern Part7 D_800BA134[];
extern Part6 D_800BA3F4[];
extern Part3 D_800BAADC[];
extern Part5 D_800BABAC[];
extern Part8 D_800BACCC[];
extern Part5 D_800BAFCC[];
extern Wpn D_800BB300[];
extern Wpn D_800BB3C0[];
extern Wpn D_800BB540[];

extern void func_800506EC(s32);
extern UiObj *func_8005B648(s16 x, s16 y, s16 w, s16 h, s16 kind, s16 a, s16 b);
extern void func_8005D80C(s16, void *, u8, s16, s32, s16);
extern void func_80074F5C(s32 n, char *buf);
void func_80075898(UiObj *o);

void func_80075898(UiObj *o)
{
    AsmWork *w;
    char **lst;
    s16 i;
    s16 y;
    u16 k;
    u16 n;
    u8 j;
    u8 f;

    w = &((AsmObj *)o)->w;
    if ((D_801A2568 & 0x10) && !(D_801A256C & 0x10)) {
        if (((AsmObj *)o)->w.bAE) {
            func_800506EC(10);
            obj_find_by_id(9)->kill = 1;
            ((AsmObj *)o)->w.bAE = 0;
        } else {
            *(s16 *)((u8 *)func_8005B648(48, 52, 228, 144, 9, 1, 0) + 0x8A) = 1;
            ((AsmObj *)o)->w.bAE = 1;
        }
    }
    if (!w->bAE)
        return;

    func_800506EC(10);
    lst = D_800BBA58[w->h90];
    if (w->h90 == 4 && w->hA6 != 0xFF && D_800B91F4[w->hA6].b36 == 0) {
        i = 0;
        y = 8;
        do {
            func_8005D80C(1, lst[i], 6, 8, y, 10);
            y += 8;
            i++;
        } while (i < 13);
    } else if (w->h90 == 6 && D_800BA3F4[w->hA6].b2C == 0) {
        i = 0;
        y = 8;
        do {
            func_8005D80C(1, lst[i], 6, 8, y, 10);
            y += 8;
            i++;
        } while (i < 6);
    } else {
        i = 0;
        y = 8;
        for (; lst[i] != 0; i++) {
            func_8005D80C(1, lst[i], 6, 8, y, 10);
            y += 8;
        }
    }

    if (w->h90 == 7) {
        for (i = 0; D_800BBA34[i] != 0; i++) {
            func_8005D80C(1, D_800BBA34[i], 6, 8, y, 10);
            y += 8;
        }
    } else if (w->h90 == 2 && w->hA6 >= 8 && w->hA6 != 0xFF) {
        for (i = 0; D_800BBA34[i] != 0; i++) {
            func_8005D80C(1, D_800BBA34[i], 6, 8, y, 10);
            y += 8;
        }
    } else if (w->h90 == 6) {
        k = w->hA6;
        if (k < 24 || k > 30 || k == 0xFF) {
            for (i = 0; D_800BBA34[i] != 0; i++) {
                func_8005D80C(1, D_800BBA34[i], 6, 8, y, 10);
                y += 8;
            }
        }
    }

    switch (w->h90) {
    case 4:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800B91F4[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800B91F4[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800B91F4[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800B91F4[w->hA6].h2C, D_8017EE80[3]);
            func_80074F5C(D_800B91F4[w->hA6].h2E, D_8017EE80[4]);
            func_80074F5C(D_800B91F4[w->hA6].h30, D_8017EE80[5]);
            func_80074F5C(D_800B91F4[w->hA6].h38, D_8017EE80[6]);
            func_8005D80C(1, D_800B91F4[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800B91F4[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 48, 10);
            func_8005D80C(1, D_8017EE80[4], 6, 122, 56, 10);
            func_8005D80C(1, D_8017EE80[5], 6, 122, 64, 10);
            func_8005D80C(1, D_800BBACC[D_800B91F4[w->hA6].b32], 6, 122, 72, 10);
            func_8005D80C(1, D_800BBAD8[D_800B91F4[w->hA6].b33], 6, 122, 80, 10);
            func_8005D80C(1, D_800BBA84[D_800B91F4[w->hA6].b34], 6, 122, 88, 10);
            func_8005D80C(1, D_800BBA84[D_800B91F4[w->hA6].b35], 6, 122, 96, 10);
            func_8005D80C(1, D_800BBA84[D_800B91F4[w->hA6].b36], 6, 122, 104, 10);
            if (w->hA6 == 0xFF || D_800B91F4[w->hA6].b36 != 0) {
                func_8005D80C(1, D_8017EE80[6], 6, 122, 112, 10);
                func_8005D80C(1, D_800BBA8C[D_800B91F4[w->hA6].b3B], 6, 122, 120, 10);
            }
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
            func_8005D80C(1, "------", 6, 122, 64, 10);
            func_8005D80C(1, "------", 6, 122, 72, 10);
            func_8005D80C(1, "------", 6, 122, 80, 10);
            func_8005D80C(1, "------", 6, 122, 88, 10);
            func_8005D80C(1, "------", 6, 122, 96, 10);
            func_8005D80C(1, "------", 6, 122, 104, 10);
            if (D_800B91F4[w->hA6].b36 != 0) {
                func_8005D80C(1, "------", 6, 122, 112, 10);
                func_8005D80C(1, "------", 6, 122, 120, 10);
            }
        }
        break;
    case 0:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800B944C[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800B944C[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800B944C[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800B944C[w->hA6].h2C, D_8017EE80[3]);
            func_80074F5C(D_800B944C[w->hA6].h2E, D_8017EE80[4]);
            func_80074F5C(D_800B944C[w->hA6].h30, D_8017EE80[5]);
            func_80074F5C(D_800B944C[w->hA6].h32, D_8017EE80[6]);
            func_80074F5C(D_800B944C[w->hA6].b34, D_8017EE80[7]);
            func_80074F5C(D_800B944C[w->hA6].b35, D_8017EE80[8]);
            func_80074F5C(D_800B944C[w->hA6].b36, D_8017EE80[9]);
            func_8005D80C(1, D_800B944C[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800B944C[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 48, 10);
            func_8005D80C(1, D_8017EE80[4], 6, 122, 56, 10);
            func_8005D80C(1, D_8017EE80[5], 6, 122, 64, 10);
            func_8005D80C(1, D_8017EE80[6], 6, 122, 72, 10);
            func_8005D80C(1, D_8017EE80[7], 6, 122, 80, 10);
            func_8005D80C(1, D_8017EE80[8], 6, 122, 88, 10);
            func_8005D80C(1, D_8017EE80[9], 6, 122, 96, 10);
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
            func_8005D80C(1, "------", 6, 122, 64, 10);
            func_8005D80C(1, "------", 6, 122, 72, 10);
            func_8005D80C(1, "------", 6, 122, 80, 10);
            func_8005D80C(1, "------", 6, 122, 88, 10);
            func_8005D80C(1, "------", 6, 122, 96, 10);
        }
        break;
    case 1:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800B94F4[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800B94F4[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800B94F4[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800B94F4[w->hA6].h2C, D_8017EE80[3]);
            func_80074F5C(D_800B94F4[w->hA6].h2E, D_8017EE80[4]);
            func_80074F5C(D_800B94F4[w->hA6].h30, D_8017EE80[5]);
            func_80074F5C(D_800B94F4[w->hA6].h32, D_8017EE80[6]);
            func_80074F5C(D_800B94F4[w->hA6].h36, D_8017EE80[7]);
            func_80074F5C(D_800B94F4[w->hA6].h40, D_8017EE80[8]);
            func_8005D80C(1, D_800B94F4[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800B94F4[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 48, 10);
            func_8005D80C(1, D_8017EE80[4], 6, 122, 56, 10);
            func_8005D80C(1, D_8017EE80[5], 6, 122, 64, 10);
            func_8005D80C(1, D_8017EE80[6], 6, 122, 72, 10);
            func_8005D80C(1, D_8017EE80[7], 6, 122, 80, 10);
            func_8005D80C(1, D_8017EE80[8], 6, 122, 88, 10);
            func_8005D80C(1, D_800BBA84[D_800B94F4[w->hA6].b42], 6, 122, 96, 10);
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
            func_8005D80C(1, "------", 6, 122, 64, 10);
            func_8005D80C(1, "------", 6, 122, 72, 10);
            func_8005D80C(1, "------", 6, 122, 80, 10);
            func_8005D80C(1, "------", 6, 122, 88, 10);
            func_8005D80C(1, "------", 6, 122, 96, 10);
        }
        break;
    case 2:
        n = w->hA6;
        if (n != 0xFF) {
            j = n - 8;
            func_80074F5C(D_800B9DF4[n].price, D_8017EE80[0]);
            func_80074F5C(D_800B9DF4[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800B9DF4[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800B9DF4[w->hA6].h2C, D_8017EE80[3]);
            func_80074F5C(D_800B9DF4[w->hA6].h2E, D_8017EE80[4]);
            func_80074F5C(D_800B9DF4[w->hA6].h30, D_8017EE80[5]);
            if (w->hA6 >= 8) {
                func_80074F5C(D_800BB300[j].h14, D_8017EE80[6]);
                func_80074F5C(D_800BB300[j].h08, D_8017EE80[7]);
                func_80074F5C(D_800BB300[j].h0C, D_8017EE80[8]);
                func_80074F5C(D_800BB300[j].h0E, D_8017EE80[9]);
                func_80074F5C(D_800BB300[j].h0E, D_8017EE80[10]);
                func_80074F5C(D_800BB300[j].b10, D_8017EE80[11]);
                func_80074F5C(D_800BB300[j].b11, D_8017EE80[12]);
            }
            func_8005D80C(1, D_800B9DF4[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800B9DF4[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 48, 10);
            func_8005D80C(1, D_8017EE80[4], 6, 122, 56, 10);
            func_8005D80C(1, D_8017EE80[5], 6, 122, 64, 10);
            if (w->hA6 >= 8) {
                func_8005D80C(1, D_800BBAA4[D_800BB300[j].b06], 6, 122, 72, 10);
                func_8005D80C(1, D_8017EE80[6], 6, 122, 80, 10);
                func_8005D80C(1, D_8017EE80[7], 6, 122, 88, 10);
                f = D_800BB300[j].b12;
                if (f & 0x10)
                    func_8005D80C(1, D_800BBA9C, 6, 122, 96, 10);
                else if (f & 0x20)
                    func_8005D80C(1, D_800BBAA0, 6, 122, 96, 10);
                else
                    func_8005D80C(1, "------", 6, 122, 96, 10);
                func_8005D80C(1, D_8017EE80[8], 6, 122, 104, 10);
                func_8005D80C(1, D_8017EE80[9], 6, 122, 112, 10);
                func_8005D80C(1, D_8017EE80[11], 6, 122, 120, 10);
                func_8005D80C(1, D_8017EE80[12], 6, 122, 128, 10);
            }
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
            func_8005D80C(1, "------", 6, 122, 64, 10);
        }
        break;
    case 7:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800BA134[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800BA134[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800BA134[w->hA6].h2A, D_8017EE80[9]);
            func_80074F5C(D_800BB3C0[w->hA6].h14, D_8017EE80[2]);
            func_80074F5C(D_800BB3C0[w->hA6].h08, D_8017EE80[3]);
            func_80074F5C(D_800BB3C0[w->hA6].h0C, D_8017EE80[4]);
            func_80074F5C(D_800BB3C0[w->hA6].h0E, D_8017EE80[5]);
            func_80074F5C(D_800BB3C0[w->hA6].b10, D_8017EE80[7]);
            func_80074F5C(D_800BB3C0[w->hA6].b11, D_8017EE80[8]);
            func_8005D80C(1, D_800BA134[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800BA134[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[9], 6, 122, 40, 10);
            func_8005D80C(1, D_800BBAA4[D_800BB3C0[w->hA6].b06], 6, 122, 48, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 56, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 64, 10);
            f = D_800BB3C0[w->hA6].b12;
            if (f & 0x10)
                func_8005D80C(1, D_800BBA9C, 6, 122, 72, 10);
            else if (f & 0x20)
                func_8005D80C(1, D_800BBAA0, 6, 122, 72, 10);
            else
                func_8005D80C(1, "------", 6, 122, 72, 10);
            func_8005D80C(1, D_8017EE80[4], 6, 122, 80, 10);
            func_8005D80C(1, D_8017EE80[5], 6, 122, 88, 10);
            func_8005D80C(1, D_8017EE80[7], 6, 122, 96, 10);
            func_8005D80C(1, D_8017EE80[8], 6, 122, 104, 10);
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
            func_8005D80C(1, "------", 6, 122, 64, 10);
            func_8005D80C(1, "------", 6, 122, 72, 10);
            func_8005D80C(1, "------", 6, 122, 80, 10);
            func_8005D80C(1, "------", 6, 122, 88, 10);
            func_8005D80C(1, "------", 6, 122, 96, 10);
            func_8005D80C(1, "------", 6, 122, 104, 10);
        }
        break;
    case 6:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800BA3F4[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800BA3F4[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800BA3F4[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800BA3F4[w->hA6].h2E, D_8017EE80[3]);
            if (w->hA6 < 24 || w->hA6 > 30) {
                func_80074F5C(D_800BB540[w->hA6].h14, D_8017EE80[4]);
                func_80074F5C(D_800BB540[w->hA6].h08, D_8017EE80[5]);
                func_80074F5C(D_800BB540[w->hA6].h0C, D_8017EE80[6]);
                func_80074F5C(D_800BB540[w->hA6].h0E, D_8017EE80[7]);
                func_80074F5C(D_800BB540[w->hA6].b10, D_8017EE80[9]);
                func_80074F5C(D_800BB540[w->hA6].b11, D_8017EE80[10]);
            }
            func_8005D80C(1, D_800BA3F4[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800BA3F4[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_800BBA84[D_800BA3F4[w->hA6].b2C], 6, 122, 48, 10);
            if (w->hA6 < 24 || w->hA6 > 30) {
                func_8005D80C(1, D_800BBAA4[D_800BB540[w->hA6].b06], 6, 122, 56, 10);
                func_8005D80C(1, D_8017EE80[4], 6, 122, 64, 10);
                func_8005D80C(1, D_8017EE80[5], 6, 122, 72, 10);
                f = D_800BB540[w->hA6].b12;
                if (f & 0x10)
                    func_8005D80C(1, D_800BBA9C, 6, 122, 80, 10);
                else if (f & 0x20)
                    func_8005D80C(1, D_800BBAA0, 6, 122, 80, 10);
                else
                    func_8005D80C(1, "------", 6, 122, 80, 10);
                func_8005D80C(1, D_8017EE80[6], 6, 122, 88, 10);
                func_8005D80C(1, D_8017EE80[7], 6, 122, 96, 10);
                func_8005D80C(1, D_8017EE80[9], 6, 122, 104, 10);
                func_8005D80C(1, D_8017EE80[10], 6, 122, 112, 10);
            } else {
                func_8005D80C(1, D_8017EE80[3], 6, 122, 56, 10);
                func_8005D80C(1, D_800BBA8C[D_800BA3F4[w->hA6].b31], 6, 122, 64, 10);
            }
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
            func_8005D80C(1, "------", 6, 122, 64, 10);
            func_8005D80C(1, "------", 6, 122, 72, 10);
            func_8005D80C(1, "------", 6, 122, 80, 10);
            func_8005D80C(1, "------", 6, 122, 88, 10);
            func_8005D80C(1, "------", 6, 122, 96, 10);
            func_8005D80C(1, "------", 6, 122, 104, 10);
            func_8005D80C(1, "------", 6, 122, 112, 10);
            func_8005D80C(1, "------", 6, 122, 120, 10);
            func_8005D80C(1, "------", 6, 122, 128, 10);
        }
        break;
    case 3:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800BAADC[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800BAADC[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800BAADC[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800BAADC[w->hA6].h2C, D_8017EE80[3]);
            func_80074F5C(D_800BAADC[w->hA6].h30, D_8017EE80[4]);
            func_8005D80C(1, D_800BAADC[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800BAADC[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 48, 10);
            func_8005D80C(1, D_8017EE80[4], 6, 122, 56, 10);
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
        }
        break;
    case 5:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800BABAC[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800BABAC[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800BABAC[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800BABAC[w->hA6].h2C, D_8017EE80[3]);
            func_80074F5C(D_800BABAC[w->hA6].h2E, D_8017EE80[4]);
            func_8005D80C(1, D_800BABAC[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800BABAC[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 48, 10);
            func_8005D80C(1, D_8017EE80[4], 6, 122, 56, 10);
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
        }
        break;
    case 8:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800BACCC[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800BACCC[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800BACCC[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800BACCC[w->hA6].b2C, D_8017EE80[3]);
            func_8005D80C(1, D_800BACCC[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800BACCC[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 48, 10);
            func_8005D80C(1, D_800BBAB8[D_800BACCC[w->hA6].b2D], 6, 122, 56, 10);
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
        }
        break;
    case 10:
        if (w->hA6 != 0xFF) {
            func_80074F5C(D_800BAFCC[w->hA6].price, D_8017EE80[0]);
            func_80074F5C(D_800BAFCC[w->hA6].h28, D_8017EE80[1]);
            func_80074F5C(D_800BAFCC[w->hA6].h2A, D_8017EE80[2]);
            func_80074F5C(D_800BAFCC[w->hA6].h2C, D_8017EE80[3]);
            func_80074F5C(D_800BAFCC[w->hA6].h2E, D_8017EE80[4]);
            func_8005D80C(1, D_800BAFCC[w->hA6].name, 6, 122, 8, 10);
            func_8005D80C(1, D_800BAFCC[w->hA6].maker, 6, 122, 16, 10);
            func_8005D80C(1, D_8017EE80[0], 6, 122, 24, 10);
            func_8005D80C(1, D_8017EE80[1], 6, 122, 32, 10);
            func_8005D80C(1, D_8017EE80[2], 6, 122, 40, 10);
            func_8005D80C(1, D_8017EE80[3], 6, 122, 48, 10);
            func_8005D80C(1, D_8017EE80[4], 6, 122, 56, 10);
        } else {
            func_8005D80C(1, "------", 6, 122, 8, 10);
            func_8005D80C(1, "------", 6, 122, 16, 10);
            func_8005D80C(1, "------", 6, 122, 24, 10);
            func_8005D80C(1, "------", 6, 122, 32, 10);
            func_8005D80C(1, "------", 6, 122, 40, 10);
            func_8005D80C(1, "------", 6, 122, 48, 10);
            func_8005D80C(1, "------", 6, 122, 56, 10);
        }
        break;
    }
}
