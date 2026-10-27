/* ovl201 0x80090E88  size 11160  wave13 a2: structured rewrite of the m2c sweep draft.
 * Shop/garage part spec sheet: draws the label column (x 8) for part kind `kind`
 * and the value column (x 122) for part `id` of that kind; id 0xFF draws "------". */
#include "ac1/ac1.h"

extern void func_800506EC(s32);
UiObj *func_8005D80C(s32 layer, char *s, s32 x, s32 w, s32 y, s32 kind);
extern void func_80090D60(s32 n, char *buf);

typedef struct { char **t[11]; } LabelSets;
extern LabelSets D_8004CD90;            /* rodata: label lists per kind (+ kind 9 id>=16) */
extern char D_8004CDBC[];               /* "------" */
extern char *D_800BBA34[];              /* extra label list, 0-terminated */

/* text buffers: 10-byte slots, all reached as D_8017F358 + k (cse derives them from one base) */
extern char D_8017F358[];

/* string tables indexed by u8 fields */
extern char *D_800BBA84[];
extern char *D_800BBA8C[];
extern char *D_800BBAA4[];
extern char *D_800BBAB8[];
extern char *D_800BBACC[];
extern char *D_800BBAD8[];
extern char *D_800BBA9C;
extern char *D_800BBAA0;

typedef struct {                        /* kind 0, 0x3C */
    char name[0x12];
    char name2[0x12];
    s32 price;                          /* +0x24 */
    u16 v28, v2A, v2C, v2E, v30;        /* +0x28 */
    u8 i32, i33, i34, i35, i36;         /* +0x32 */
    u8 pad37;
    u16 v38;                            /* +0x38 */
    u8 pad3A;
    u8 i3B;                             /* +0x3B */
} Part0;
extern Part0 D_800B91F4[];

typedef struct {                        /* kind 1, 0x38 */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u16 v28, v2A, v2C, v2E, v30, v32;
    u8 b34, b35, b36;
    u8 pad37;
} Part1;
extern Part1 D_800B944C[];

typedef struct {                        /* kind 3, 0x48 */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u16 v28, v2A, v2C, v2E, v30, v32;
    u8 pad34[2];
    u16 v36;                            /* +0x36 */
    u8 pad38[8];
    u16 v40;                            /* +0x40 */
    u8 i42;                             /* +0x42 */
    u8 pad43[5];
} Part3;
extern Part3 D_800B94F4[];

typedef struct {                        /* kind 2, 0x34 */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u16 v28, v2A, v2C, v2E, v30;
    u8 pad32[2];
} Part2;
extern Part2 D_800B9DF4[];

typedef struct {                        /* kind 2/8/9 extra block, 0x18 */
    u8 pad0[6];
    u8 i6;                              /* +0x06 */
    u8 pad7;
    u16 v8;                             /* +0x08 */
    u8 padA[2];
    u16 vC;                             /* +0x0C */
    s16 vE;                             /* +0x0E */
    u8 b10, b11;                        /* +0x10 */
    u8 flags;                           /* +0x12 */
    u8 pad13;
    u16 v14;                            /* +0x14 */
    u8 pad16[2];
} Extra;
extern Extra D_800BB300[];
extern Extra D_800BB3C0[];
extern Extra D_800BB540[];

typedef struct {                        /* kind 9 id>=16, 0x34 */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u16 v28, v2A, v2C;
    u8 pad2E[2];
    u16 v30;
    u8 pad32[2];
} Part9b;
extern Part9b D_800BAADC[];

typedef struct {                        /* kind 9 id<16, 0x2C */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u16 v28, v2A;
} Part9;
extern Part9 D_800BA134[];

typedef struct {                        /* kind 8, 0x34 */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u16 v28, v2A;
    u8 i2C;
    u8 pad2D;
    u16 v2E;
    u8 pad30;
    u8 i31;
    u8 pad32[2];
} Part8;
extern Part8 D_800BA3F4[];

typedef struct {                        /* kind 7 / 4, 0x30 */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u16 v28, v2A, v2C, v2E;
} Part7;
extern Part7 D_800BABAC[];
extern Part7 D_800BAFCC[];

typedef struct {                        /* kind 5, 0x60 */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u16 v28, v2A;
    u8 b2C, i2D;
    u8 pad2E[0x32];
} Part5;
extern Part5 D_800BACCC[];

typedef struct {                        /* kind 6, 0x2C */
    char name[0x12];
    char name2[0x12];
    s32 price;
    u8 b28;
    u8 pad29[3];
} Part6;
extern Part6 D_800BB11C[];

#define VAL(s, y) func_8005D80C(1, (s), 6, 122, (y), 12)
#define DASH(y) VAL(D_8004CDBC, (y))

void func_80090E88(s8 kind, s16 id)
{
    LabelSets sets;
    char **labels;
    s16 i;
    s16 y;

    sets = D_8004CD90;
    func_800506EC(12);

    if (kind == 9 && id >= 16) {
        labels = sets.t[10];
    } else {
        labels = sets.t[kind];
    }

    /* label column */
    if (kind == 0 && D_800B91F4[id].i36 == 0) {
        i = 0;
        y = 8;
        do {
            func_8005D80C(1, labels[i], 6, 8, y, 12);
            y += 8;
            i++;
        } while (i < 13);
    } else if (kind == 8 && (u16)(id - 24) >= 7) {
        i = 0;
        y = 8;
        do {
            func_8005D80C(1, labels[i], 6, 8, y, 12);
            y += 8;
            i++;
        } while (i < 6);
    } else {
        i = 0;
        y = 8;
        while (labels[i] != 0) {
            func_8005D80C(1, labels[i], 6, 8, y, 12);
            y += 8;
            i++;
        }
    }

    if (kind == 9 && id < 16) {
        for (i = 0; D_800BBA34[i] != 0; i++) {
            func_8005D80C(1, D_800BBA34[i], 6, 8, y, 12);
            y += 8;
        }
    } else if (kind == 2 && id >= 8) {
        for (i = 0; D_800BBA34[i] != 0; i++) {
            func_8005D80C(1, D_800BBA34[i], 6, 8, y, 12);
            y += 8;
        }
    } else if (kind == 8 && (u16)(id - 24) >= 7) {
        for (i = 0; D_800BBA34[i] != 0; i++) {
            func_8005D80C(1, D_800BBA34[i], 6, 8, y, 12);
            y += 8;
        }
    }

    /* value column */
    if (id == 0xFF) {
        switch (kind) {
        case 0:
            DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20); DASH(0x28); DASH(0x30);
            DASH(0x38); DASH(0x40); DASH(0x48); DASH(0x50); DASH(0x58); DASH(0x60);
            DASH(0x68);
            if (D_800B91F4[id].i36 != 0) {
                DASH(0x70);
                DASH(0x78);
            }
            break;
        case 1:
            DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20); DASH(0x28); DASH(0x30);
            DASH(0x38); DASH(0x40); DASH(0x48); DASH(0x50); DASH(0x58); DASH(0x60);
            break;
        case 3:
            DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20); DASH(0x28); DASH(0x30);
            DASH(0x38); DASH(0x40); DASH(0x48); DASH(0x50); DASH(0x58); DASH(0x60);
            break;
        case 2:
            DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20); DASH(0x28); DASH(0x30);
            DASH(0x38); DASH(0x40);
            break;
        case 9:
            if (kind == 9 && id >= 16) {
                DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20); DASH(0x28); DASH(0x30);
                DASH(0x38);
            } else {
                DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20); DASH(0x28); DASH(0x30);
                DASH(0x38); DASH(0x40); DASH(0x48); DASH(0x50); DASH(0x58); DASH(0x60);
                DASH(0x68);
            }
            break;
        case 8:
            DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20); DASH(0x28); DASH(0x30);
            DASH(0x38); DASH(0x40); DASH(0x48); DASH(0x50); DASH(0x58); DASH(0x60);
            DASH(0x68); DASH(0x70); DASH(0x78); DASH(0x80);
            break;
        case 4:
        case 5:
        case 7:
            DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20); DASH(0x28); DASH(0x30);
            DASH(0x38);
            break;
        case 6:
            DASH(0x08); DASH(0x10); DASH(0x18); DASH(0x20);
            break;
        }
    } else {
        switch (kind) {
        case 0: {
            func_80090D60(D_800B91F4[id].price, &D_8017F358[0]);
            func_80090D60(D_800B91F4[id].v28, &D_8017F358[10]);
            func_80090D60(D_800B91F4[id].v2A, &D_8017F358[20]);
            func_80090D60(D_800B91F4[id].v2C, &D_8017F358[30]);
            func_80090D60(D_800B91F4[id].v2E, &D_8017F358[40]);
            func_80090D60(D_800B91F4[id].v30, &D_8017F358[50]);
            func_80090D60(D_800B91F4[id].v38, &D_8017F358[60]);
            VAL(D_800B91F4[id].name, 0x08);
            VAL(D_800B91F4[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            VAL(&D_8017F358[20], 0x28);
            VAL(&D_8017F358[30], 0x30);
            VAL(&D_8017F358[40], 0x38);
            VAL(&D_8017F358[50], 0x40);
            VAL(D_800BBACC[D_800B91F4[id].i32], 0x48);
            VAL(D_800BBAD8[D_800B91F4[id].i33], 0x50);
            VAL(D_800BBA84[D_800B91F4[id].i34], 0x58);
            VAL(D_800BBA84[D_800B91F4[id].i35], 0x60);
            VAL(D_800BBA84[D_800B91F4[id].i36], 0x68);
            if (D_800B91F4[id].i36 != 0) {
                VAL(&D_8017F358[60], 0x70);
                VAL(D_800BBA8C[D_800B91F4[id].i3B], 0x78);
            }
            break;
        }
        case 1: {
            func_80090D60(D_800B944C[id].price, &D_8017F358[0]);
            func_80090D60(D_800B944C[id].v28, &D_8017F358[10]);
            func_80090D60(D_800B944C[id].v2A, &D_8017F358[20]);
            func_80090D60(D_800B944C[id].v2C, &D_8017F358[30]);
            func_80090D60(D_800B944C[id].v2E, &D_8017F358[40]);
            func_80090D60(D_800B944C[id].v30, &D_8017F358[50]);
            func_80090D60(D_800B944C[id].v32, &D_8017F358[60]);
            func_80090D60(D_800B944C[id].b34, &D_8017F358[70]);
            func_80090D60(D_800B944C[id].b35, &D_8017F358[80]);
            func_80090D60(D_800B944C[id].b36, &D_8017F358[90]);
            VAL(D_800B944C[id].name, 0x08);
            VAL(D_800B944C[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            VAL(&D_8017F358[20], 0x28);
            VAL(&D_8017F358[30], 0x30);
            VAL(&D_8017F358[40], 0x38);
            VAL(&D_8017F358[50], 0x40);
            VAL(&D_8017F358[60], 0x48);
            VAL(&D_8017F358[70], 0x50);
            VAL(&D_8017F358[80], 0x58);
            VAL(&D_8017F358[90], 0x60);
            break;
        }
        case 3: {
            func_80090D60(D_800B94F4[id].price, &D_8017F358[0]);
            func_80090D60(D_800B94F4[id].v28, &D_8017F358[10]);
            func_80090D60(D_800B94F4[id].v2A, &D_8017F358[20]);
            func_80090D60(D_800B94F4[id].v2C, &D_8017F358[30]);
            func_80090D60(D_800B94F4[id].v2E, &D_8017F358[40]);
            func_80090D60(D_800B94F4[id].v30, &D_8017F358[50]);
            func_80090D60(D_800B94F4[id].v32, &D_8017F358[60]);
            func_80090D60(D_800B94F4[id].v36, &D_8017F358[70]);
            func_80090D60(D_800B94F4[id].v40, &D_8017F358[80]);
            VAL(D_800B94F4[id].name, 0x08);
            VAL(D_800B94F4[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            VAL(&D_8017F358[20], 0x28);
            VAL(&D_8017F358[30], 0x30);
            VAL(&D_8017F358[40], 0x38);
            VAL(&D_8017F358[50], 0x40);
            VAL(&D_8017F358[60], 0x48);
            VAL(&D_8017F358[70], 0x50);
            VAL(&D_8017F358[80], 0x58);
            VAL(D_800BBA84[D_800B94F4[id].i42], 0x60);
            break;
        }
        case 2: {
            s16 n = id - 8;

            func_80090D60(D_800B9DF4[id].price, &D_8017F358[0]);
            func_80090D60(D_800B9DF4[id].v28, &D_8017F358[10]);
            func_80090D60(D_800B9DF4[id].v2A, &D_8017F358[20]);
            func_80090D60(D_800B9DF4[id].v2C, &D_8017F358[30]);
            func_80090D60(D_800B9DF4[id].v2E, &D_8017F358[40]);
            func_80090D60(D_800B9DF4[id].v30, &D_8017F358[50]);
            if (id >= 8) {
                func_80090D60(D_800BB300[(u8)n].v14, &D_8017F358[60]);
                func_80090D60(D_800BB300[(u8)n].v8, &D_8017F358[70]);
                func_80090D60(D_800BB300[(u8)n].vC, &D_8017F358[80]);
                func_80090D60(D_800BB300[(u8)n].vE, &D_8017F358[90]);
                func_80090D60(D_800BB300[(u8)n].vE, &D_8017F358[100]);
                func_80090D60(D_800BB300[(u8)n].b10, &D_8017F358[110]);
                func_80090D60(D_800BB300[(u8)n].b11, &D_8017F358[120]);
            }
            VAL(D_800B9DF4[id].name, 0x08);
            VAL(D_800B9DF4[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            VAL(&D_8017F358[20], 0x28);
            VAL(&D_8017F358[30], 0x30);
            VAL(&D_8017F358[40], 0x38);
            VAL(&D_8017F358[50], 0x40);
            if (id >= 8) {
                VAL(D_800BBAA4[D_800BB300[(u8)n].i6], 0x48);
                VAL(&D_8017F358[60], 0x50);
                VAL(&D_8017F358[70], 0x58);
                if (D_800BB300[(u8)n].flags & 0x10) {
                    VAL(D_800BBA9C, 0x60);
                } else if (D_800BB300[(u8)n].flags & 0x20) {
                    VAL(D_800BBAA0, 0x60);
                } else {
                    VAL(D_8004CDBC, 0x60);
                }
                VAL(&D_8017F358[80], 0x68);
                VAL(&D_8017F358[90], 0x70);
                VAL(&D_8017F358[110], 0x78);
                VAL(&D_8017F358[120], 0x80);
            }
            break;
        }
        case 9:
            if (id >= 16) {
                func_80090D60(D_800BAADC[(u8)(id - 16)].price, &D_8017F358[0]);
                func_80090D60(D_800BAADC[(u8)(id - 16)].v28, &D_8017F358[10]);
                func_80090D60(D_800BAADC[(u8)(id - 16)].v2A, &D_8017F358[20]);
                func_80090D60(D_800BAADC[(u8)(id - 16)].v2C, &D_8017F358[30]);
                func_80090D60(D_800BAADC[(u8)(id - 16)].v30, &D_8017F358[40]);
                VAL(D_800BAADC[(u8)(id - 16)].name, 0x08);
                VAL(D_800BAADC[(u8)(id - 16)].name2, 0x10);
                VAL(&D_8017F358[0], 0x18);
                VAL(&D_8017F358[10], 0x20);
                VAL(&D_8017F358[20], 0x28);
                VAL(&D_8017F358[30], 0x30);
                VAL(&D_8017F358[40], 0x38);
            } else {
                func_80090D60(D_800BA134[id].price, &D_8017F358[0]);
                func_80090D60(D_800BA134[id].v28, &D_8017F358[10]);
                func_80090D60(D_800BA134[id].v2A, &D_8017F358[90]);
                func_80090D60(D_800BB3C0[id].v14, &D_8017F358[20]);
                func_80090D60(D_800BB3C0[id].v8, &D_8017F358[30]);
                func_80090D60(D_800BB3C0[id].vC, &D_8017F358[40]);
                func_80090D60(D_800BB3C0[id].vE, &D_8017F358[50]);
                func_80090D60(D_800BB3C0[id].b10, &D_8017F358[70]);
                func_80090D60(D_800BB3C0[id].b11, &D_8017F358[80]);
                VAL(D_800BA134[id].name, 0x08);
                VAL(D_800BA134[id].name2, 0x10);
                VAL(&D_8017F358[0], 0x18);
                VAL(&D_8017F358[10], 0x20);
                VAL(&D_8017F358[90], 0x28);
                VAL(D_800BBAA4[D_800BB3C0[id].i6], 0x30);
                VAL(&D_8017F358[20], 0x38);
                VAL(&D_8017F358[30], 0x40);
                if (D_800BB3C0[id].flags & 0x10) {
                    VAL(D_800BBA9C, 0x48);
                } else if (D_800BB3C0[id].flags & 0x20) {
                    VAL(D_800BBAA0, 0x48);
                } else {
                    VAL(D_8004CDBC, 0x48);
                }
                VAL(&D_8017F358[40], 0x50);
                VAL(&D_8017F358[50], 0x58);
                VAL(&D_8017F358[70], 0x60);
                VAL(&D_8017F358[80], 0x68);
            }
            break;
        case 8: {
            func_80090D60(D_800BA3F4[id].price, &D_8017F358[0]);
            func_80090D60(D_800BA3F4[id].v28, &D_8017F358[10]);
            func_80090D60(D_800BA3F4[id].v2A, &D_8017F358[20]);
            func_80090D60(D_800BA3F4[id].v2E, &D_8017F358[30]);
            if ((u16)(id - 24) >= 7) {
                func_80090D60(D_800BB540[id].v14, &D_8017F358[40]);
                func_80090D60(D_800BB540[id].v8, &D_8017F358[50]);
                func_80090D60(D_800BB540[id].vC, &D_8017F358[60]);
                func_80090D60(D_800BB540[id].vE, &D_8017F358[70]);
                func_80090D60(D_800BB540[id].b10, &D_8017F358[90]);
                func_80090D60(D_800BB540[id].b11, &D_8017F358[100]);
            }
            VAL(D_800BA3F4[id].name, 0x08);
            VAL(D_800BA3F4[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            VAL(&D_8017F358[20], 0x28);
            VAL(D_800BBA84[D_800BA3F4[id].i2C], 0x30);
            if ((u16)(id - 24) >= 7) {
                VAL(D_800BBAA4[D_800BB540[id].i6], 0x38);
                VAL(&D_8017F358[40], 0x40);
                VAL(&D_8017F358[50], 0x48);
                if (D_800BB540[id].flags & 0x10) {
                    VAL(D_800BBA9C, 0x50);
                } else if (D_800BB540[id].flags & 0x20) {
                    VAL(D_800BBAA0, 0x50);
                } else {
                    VAL(D_8004CDBC, 0x50);
                }
                VAL(&D_8017F358[60], 0x58);
                VAL(&D_8017F358[70], 0x60);
                VAL(&D_8017F358[90], 0x68);
                VAL(&D_8017F358[100], 0x70);
            } else {
                VAL(&D_8017F358[30], 0x38);
                VAL(D_800BBA8C[D_800BA3F4[id].i31], 0x40);
            }
            break;
        }
        case 7: {
            func_80090D60(D_800BABAC[id].price, &D_8017F358[0]);
            func_80090D60(D_800BABAC[id].v28, &D_8017F358[10]);
            func_80090D60(D_800BABAC[id].v2A, &D_8017F358[20]);
            func_80090D60(D_800BABAC[id].v2C, &D_8017F358[30]);
            func_80090D60(D_800BABAC[id].v2E, &D_8017F358[40]);
            VAL(D_800BABAC[id].name, 0x08);
            VAL(D_800BABAC[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            VAL(&D_8017F358[20], 0x28);
            VAL(&D_8017F358[30], 0x30);
            VAL(&D_8017F358[40], 0x38);
            break;
        }
        case 5: {
            func_80090D60(D_800BACCC[id].price, &D_8017F358[0]);
            func_80090D60(D_800BACCC[id].v28, &D_8017F358[10]);
            func_80090D60(D_800BACCC[id].v2A, &D_8017F358[20]);
            func_80090D60(D_800BACCC[id].b2C, &D_8017F358[30]);
            VAL(D_800BACCC[id].name, 0x08);
            VAL(D_800BACCC[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            VAL(&D_8017F358[20], 0x28);
            VAL(&D_8017F358[30], 0x30);
            VAL(D_800BBAB8[D_800BACCC[id].i2D], 0x38);
            break;
        }
        case 4: {
            func_80090D60(D_800BAFCC[id].price, &D_8017F358[0]);
            func_80090D60(D_800BAFCC[id].v28, &D_8017F358[10]);
            func_80090D60(D_800BAFCC[id].v2A, &D_8017F358[20]);
            func_80090D60(D_800BAFCC[id].v2C, &D_8017F358[30]);
            func_80090D60(D_800BAFCC[id].v2E, &D_8017F358[40]);
            VAL(D_800BAFCC[id].name, 0x08);
            VAL(D_800BAFCC[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            VAL(&D_8017F358[20], 0x28);
            VAL(&D_8017F358[30], 0x30);
            VAL(&D_8017F358[40], 0x38);
            break;
        }
        case 6: {
            func_80090D60(D_800BB11C[id].price, &D_8017F358[0]);
            func_80090D60(D_800BB11C[id].b28, &D_8017F358[10]);
            VAL(D_800BB11C[id].name, 0x08);
            VAL(D_800BB11C[id].name2, 0x10);
            VAL(&D_8017F358[0], 0x18);
            VAL(&D_8017F358[10], 0x20);
            break;
        }
        }
    }
}
