#include "ac1/ac1.h"

/* part tables: name first, then u16 stats; stride differs per category */
#define PARTL(n, sz) typedef struct { char name[0x28]; u16 weight; u16 s2a; u16 s2c; u16 s2e; u16 s30; u16 load; u8 pad[sz - 0x34]; } n
#define PART(n, sz) typedef struct { char name[0x28]; u16 weight; u8 pad[sz - 0x2A]; } n
PART(Part3C, 0x3C);
PARTL(Part38, 0x38);
PARTL(Part48, 0x48);
PART(Part34, 0x34);
PART(Part30, 0x30);
PART(Part60, 0x60);
typedef struct { char name[0x28]; u16 weight; u16 s2a; } Part2C;

extern u8 D_80031A58[];
extern u8 D_80039D20;
extern u8 D_80039D22;
extern s32 D_8004AC38;
extern Part3C D_800B91F4[];
extern Part38 D_800B944C[];
extern Part48 D_800B94F4[];
extern Part34 D_800B9DF4[];
extern Part2C D_800BA134[];
extern Part34 D_800BA3F4[];
extern Part34 D_800BAADC[];
extern Part30 D_800BABAC[];
extern Part60 D_800BACCC[];
extern Part30 D_800BAFCC[];
char *strcpy(char *, const char *);
void func_80071A18(void);

s32 func_800714FC(char *buf, char *name)
{
    u16 w;

    strcpy(buf, name);
    if (D_80039D20 != 0)
        strcpy(buf, "\x82q\x82\x85\x82\x82\x82\x85\x82\x8c>");
    if (D_80031A58[2] == 255)
        return 0;
    if (D_80031A58[0] == 255)
        return 0;
    if (D_80031A58[3] == 255)
        return 0;
    if (D_80031A58[8] == 255)
        return 0;
    if (D_80031A58[9] == 255)
        return 0;
    if (D_80031A58[11] < 27 && D_80031A58[1] == 255)
        return 0;
    func_80071A18();
    if (D_8004AC38 < 0)
        return 0;
    w = 0;
    if (D_80031A58[3] != 255) {
        w = D_800B9DF4[D_80031A58[3]].weight;
        if (D_80031A58[3] < 8) {
            if (D_80031A58[5] != 255)
                w += D_800BA134[D_80031A58[5]].weight;
            if (D_80031A58[4] != 255)
                w += D_800BAADC[D_80031A58[4]].weight;
        }
    }
    if (D_80031A58[0] == 255 || D_800B944C[D_80031A58[0]].load < w) {
        if (D_80039D22 == 0)
            return 0;
        strcpy(buf, "Illegal machine.>");
    }
    w = D_800B944C[D_80031A58[0]].weight;
    if (D_80031A58[3] != 255)
        w += D_800B9DF4[D_80031A58[3]].weight;
    if (D_80031A58[2] != 255)
        w += D_800B91F4[D_80031A58[2]].weight;
    if (D_80031A58[8] != 255)
        w += D_800BAFCC[D_80031A58[8]].weight;
    if (D_80031A58[9] != 255)
        w += D_800BACCC[D_80031A58[9]].weight;
    if (D_80031A58[1] != 255)
        w += D_800BABAC[D_80031A58[1]].weight;
    if (D_80031A58[6] != 255)
        w += D_800BA3F4[D_80031A58[6]].weight;
    if (D_80031A58[7] != 255)
        w += D_800BA3F4[D_80031A58[7]].weight;
    if (D_80031A58[4] != 255)
        w += D_800BAADC[D_80031A58[4]].weight;
    if (D_80031A58[5] != 255)
        w += D_800BA134[D_80031A58[5]].weight;
    if (D_80031A58[11] == 255 || D_800B94F4[D_80031A58[11]].load < w) {
        if (D_80039D22 == 0)
            return 0;
        strcpy(buf, "Illegal machine.>");
    }
    return 1;
}
