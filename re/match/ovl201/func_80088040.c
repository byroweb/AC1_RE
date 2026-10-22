#include "ac1/ac1.h"

typedef struct {
    s8 count;                   /* +0x000 */
    s8 rows;                    /* +0x001 */
    u8 pad2[0x26];
    UiObj *label[5];            /* +0x028 */
    u8 pad3C[0x23F - 0x3C];
    char row[1][34];            /* +0x23F */
} NameList;

typedef struct {
    u8 pad0[0x7A];
    s8 top;                     /* +0x07A 0x801F37CA first entry shown */
    u8 pad7B[0x84 - 0x7B];
    s8 order[64];               /* +0x084 0x801F37D4 */
    u8 padC4[0x1944 - 0xC4];
    NameList *list;             /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;
extern char D_800B8B04[][12];
extern char D_800B8A74[];
extern s16 D_800B9044[9];
extern s16 D_800B8F44[9];
extern s32 func_800517C8(void);
extern void func_8008BC58(void);
extern void func_8008BDA8(void);
extern void func_80087950(void);
extern void func_8008AFA8(void);
extern void func_80087EB8(u8 id, u8 row);
extern void func_80087F58(u8 id, u8 row);
extern UiObj *func_8005CAE0(void *p, s32 x, s32 y, s32 n);
extern UiObj *func_8005CB94(void *p, s32 x, s32 y, s32 n);
extern void func_80088334(UiObj *o);

void func_80088040(UiObj *o)
{
    s32 i;

    while ((s16)func_800517C8() != 0) {
    }
    func_8008BC58();
    func_8008BDA8();
    func_80087950();
    func_8008AFA8();
    i = D_801F3750.list->rows;
    while (--i != -1) {
        func_80087EB8(D_801F3750.order[(u8)((i + D_801F3750.top) % D_801F3750.list->count)], i);
        func_8005CAE0(D_800B8B04[i], 200, (s16)(i * 24 + 5), 4)->light = 20;
    }
    DrawSync(0);
    D_800B9044[0] = 96;
    D_800B9044[1] = 96;
    D_800B9044[2] = 96;
    D_800B9044[3] = 96;
    D_800B9044[4] = 96;
    D_800B9044[5] = 96;
    D_800B9044[6] = 96;
    D_800B9044[7] = 96;
    D_800B9044[8] = 96;
    i = D_801F3750.list->rows;
    while (--i != -1) {
        func_80087F58(D_801F3750.order[(u8)((i + D_801F3750.top) % D_801F3750.list->count)], i);
        D_801F3750.list->label[i] = func_8005CB94(D_801F3750.list->row[i], 8, (s16)(i * 24 + 5), 4);
        D_801F3750.list->label[i]->light = 12;
    }
    D_800B8F44[0] = 64;
    D_800B8F44[1] = 64;
    D_800B8F44[2] = 64;
    D_800B8F44[3] = 64;
    D_800B8F44[4] = 64;
    D_800B8F44[5] = 64;
    D_800B8F44[6] = 64;
    D_800B8F44[7] = 64;
    D_800B8F44[8] = 64;
    i = D_801F3750.list->rows;
    while (--i != -1) {
        func_8005CAE0(D_800B8A74, 0, (s16)(i * 24), 4);
    }
    o->update = func_80088334;
}
