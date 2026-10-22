#include "ac1/ac1.h"

extern s32 D_8004AC38;
extern s32 D_8004AC3C;
extern s32 D_8004AC40;
extern s32 D_8004AC48;
extern s32 D_8004AC4C;
extern u16 D_8004AC50[];
extern u16 D_8004AC52;
extern u16 D_8004AC54;
extern char D_8004BEA8[];
extern char D_8004BEAC[];
extern char D_8004BEB4[];
extern char D_8004BEBC[];
extern char D_8004BEC0[];
extern char D_8004BF98[];
extern char D_8017EE48[8][7];
void func_800506EC(s32);
UiObj *func_8005D80C(s32 layer, char *s, s32 x, s32 w, s32 y, s32 kind);
void func_80074E44(char *buf, s32 n);

void func_80075520(void)
{
    UiObj *t;

    func_800506EC(8);
    func_80074E44(D_8017EE48[0], D_8004AC50[0] - D_8004AC38);
    func_80074E44(D_8017EE48[1], D_8004AC52 - D_8004AC48);
    func_80074E44(D_8017EE48[2], D_8004AC54 - D_8004AC4C);
    func_80074E44(D_8017EE48[3], D_8004AC3C);
    func_80074E44(D_8017EE48[4], D_8004AC40);
    func_80074E44(D_8017EE48[5], D_8004AC50[0]);
    func_80074E44(D_8017EE48[6], D_8004AC52);
    func_80074E44(D_8017EE48[7], D_8004AC54);

    t = func_8005D80C(1, D_8017EE48[0], 6, 0x30, 8, 8);
    func_8005D80C(1, D_8004BF98, 6, 0x56, 8, 8);
    func_8005D80C(1, D_8017EE48[5], 6, 0x54, 8, 8);
    if (D_8004AC38 < 0)
        t->light = 13;
    t = func_8005D80C(1, D_8017EE48[1], 6, 0x30, 16, 8);
    func_8005D80C(1, D_8004BF98, 6, 0x56, 16, 8);
    func_8005D80C(1, D_8017EE48[6], 6, 0x54, 16, 8);
    if (D_8004AC48 < 0)
        t->light = 13;
    t = func_8005D80C(1, D_8017EE48[2], 6, 0x30, 24, 8);
    func_8005D80C(1, D_8004BF98, 6, 0x56, 24, 8);
    func_8005D80C(1, D_8017EE48[7], 6, 0x54, 24, 8);
    if (D_8004AC4C < 0)
        t->light = 13;
    func_8005D80C(1, D_8017EE48[3], 6, 0x54, 40, 8);
    func_8005D80C(1, D_8017EE48[4], 6, 0x54, 48, 8);
    func_8005D80C(1, D_8004BEA8, 6, 8, 8, 8);
    func_8005D80C(1, D_8004BEAC, 6, 8, 16, 8);
    func_8005D80C(1, D_8004BEB4, 6, 8, 24, 8);
    func_8005D80C(1, D_8004BEBC, 6, 8, 40, 8);
    func_8005D80C(1, D_8004BEC0, 6, 8, 48, 8);
}
