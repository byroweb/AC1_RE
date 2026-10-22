#include "ac1/ac1.h"
/* ovl201 0x8008CD38 - shop BUY/SELL menu setup: draw the two words, make
 * their labels, load the menu colour rows, make the two cursor objects and
 * continue with func_8008D560. */

typedef struct {
    u8 pad00[0xBC];
    UiObj *cursor[2];   /* +0xBC */
    u8 padC4[0x1C];
    UiObj *label[2];    /* +0xE0 */
} ShopState;
extern ShopState D_8019C090;
extern s16 D_800B8F84[][16];
extern u8 D_800B8B78[];
extern u8 D_800B8BE4[][12];
extern void func_80094E08(void);
extern void func_80095130(void);
extern void func_8005D15C(char *s, s32 x, s32 y);
extern UiObj *func_8005CAE0(void *res, s32 x, s32 y, s32 z);
extern void func_8008D560(UiObj *o);

void func_8008CD38(UiObj *o)
{
    char word[2][5] = { "BUY", "SELL" };
    UiObj *l;
    s32 i;

    func_80094E08();
    func_80095130();
    for (i = 1; i != -1; i--) {
        func_8005D15C(word[i], 0x3F0, (s16)(i * 16 + 0xB0));
        l = func_8005CAE0(D_800B8BE4[i], 8, (s16)(i * 24 + 5), 4);
        D_8019C090.label[i] = l;
        l->light = 20;
    }
    DrawSync(0);
    D_800B8F84[6][0] = 0x80;
    D_800B8F84[6][1] = 0x80;
    D_800B8F84[6][2] = 0x80;
    D_800B8F84[6][3] = 0x80;
    D_800B8F84[6][4] = 0x80;
    D_800B8F84[6][5] = 0x80;
    D_800B8F84[6][6] = 0x80;
    D_800B8F84[6][7] = 0x80;
    D_800B8F84[6][8] = 0x80;
    D_800B8F84[2][0] = 0x20;
    D_800B8F84[2][1] = 0x20;
    D_800B8F84[2][2] = 0x20;
    D_800B8F84[2][3] = 0x20;
    D_800B8F84[2][4] = 0x20;
    D_800B8F84[2][5] = 0x20;
    D_800B8F84[2][6] = 0x20;
    D_800B8F84[2][7] = 0x20;
    D_800B8F84[2][8] = 0x20;
    D_800B8F84[1][0] = 0xFF;
    D_800B8F84[1][1] = 0xFF;
    D_800B8F84[1][2] = 0xFF;
    D_800B8F84[1][3] = 0x40;
    D_800B8F84[1][4] = 0x40;
    D_800B8F84[1][5] = 0x40;
    D_800B8F84[1][6] = 0x20;
    D_800B8F84[1][7] = 0x20;
    D_800B8F84[1][8] = 0x20;
    D_800B8F84[0][0] = 0x20;
    D_800B8F84[0][1] = 0x20;
    D_800B8F84[0][2] = 0x20;
    D_800B8F84[0][3] = 0x40;
    D_800B8F84[0][4] = 0x40;
    D_800B8F84[0][5] = 0x40;
    D_800B8F84[0][6] = 0xFF;
    D_800B8F84[0][7] = 0xFF;
    D_800B8F84[0][8] = 0xFF;
    D_8019C090.cursor[0] = func_8005CAE0(D_800B8B78, 0, 0, 4);
    D_8019C090.cursor[1] = func_8005CAE0(D_800B8B78, 0, 24, 4);
    o->update = func_8008D560;
}
