#include "ac1/ac1.h"

/* "check performance" screen: AP / weight / price header and the
 * performance table of the assembled AC, then the grade. */

typedef struct { s32 price; u8 pad[40]; } OptRow;                 /* 44 */
typedef struct { u16 weight; u8 pad[22]; } Row24;               /* 24 */
typedef struct { u16 weight; u8 pad[50]; } Row52w;              /* 52 */
typedef struct { u16 off; u16 def; u8 pad[56]; } Row60;         /* 60 */
typedef struct { u16 off; u16 def; u8 pad[48]; } Row52;         /* 52 */
typedef struct { u16 off; u16 def; u8 pad[52]; } Row56;         /* 56 */
typedef struct {
    u16 off;            /* +0x00 */
    u16 def;            /* +0x02 */
    u8 pad04[4];
    u16 stab;           /* +0x08 */
    u16 mob;            /* +0x0A */
    u8 pad0c[6];
    u16 boost;          /* +0x12 */
    u8 pad14[0x34];
} Row72;                                                        /* 72 */
typedef struct { u16 speed; u8 pad[46]; } Row48;                /* 48 */
typedef struct {
    s16 x;
    s16 y;
    s16 pad04[2];
    s16 z;
    s16 pad0a;
} Sub12;
typedef struct {
    u8 a;               /* +0x00 */
    u8 pad01;
    u16 b;              /* +0x02 */
    Sub12 sub[4];       /* +0x04 */
    u8 pad34[0x2C];
} Row96;                                                        /* 96 */
typedef struct { u8 pad[0x1C]; u16 radar; } Sys411F4;

extern OptRow D_800BB140[];
extern Row24 D_800BB314[];
extern Row24 D_800BB3D4[];
extern Row52w D_800BAB0C[];
extern Row24 D_800BB554[];
extern Row60 D_800B9222[];
extern Row52 D_800B9E22[];
extern Row72 D_800B9522[];
extern Row56 D_800B947A[];
extern Row48 D_800BABD8[];
extern Row96 D_800BACF8[];
extern Sys411F4 D_800411F4;

extern u8 D_80031A58;
extern u8 D_80031A59;
extern u8 D_80031A5A;
extern u8 D_80031A5B;
extern u8 D_80031A5C;
extern u8 D_80031A5D;
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern u8 D_80031A61;
extern u8 D_80031A63;
extern char D_80031BD4[];
extern u16 D_80039CA0;
typedef struct { s32 ap; s32 weight; s32 price; } AcTotals;
extern AcTotals D_8004AC3C;
extern char D_8004CA68[];
extern char D_8004CA70[];
extern char D_8004CA78[];
extern char D_8004CA88[];
extern char D_8004CA94[];
extern char D_8004BEBC[];
extern char D_8004BEC0[];
extern char D_8004CAB4[];
extern char D_8004CABC[];
extern char D_8004CACC[];
extern char D_8004CADC[];
extern char D_8004CAE8[];
extern char D_8004CAF4[];
extern char D_8004CB04[];
extern char D_8004CB14[];
extern char D_8004CB24[];
extern char D_8004CB34[];
extern char D_8004CB40[];
extern char D_8004CB50[];
extern char D_8004CB58[];
extern char D_8017EF0C[2][9];
extern char D_8017EF20[7];
extern char D_8017EF30[11][7];

extern UiObj *func_8005CB94(char *s, s32 a, s32 b, s32 c);
extern UiObj *func_8005D80C(s32 a, char *s, s32 b, s32 c, s32 d, s32 e);
extern void func_80071A18(void);
extern void func_80085320(u8 digits, char *buf, s32 val);
extern void func_8008630C(UiObj *o);
extern void func_8009C964(s32 a);
/* soft-float helpers called directly: this cc1 build cannot emit double
 * constants as immediates (it writes broken .rdata words), so the
 * constants 2^32 and 1.15 are passed as their IEEE bit patterns */
extern double __adddf3(double a, long long b);
extern double __muldf3(double a, long long b);

void func_80085640(UiObj *o)
{
    char *grade[5] = { D_8004CA68, D_8004CA70, D_8004CA78, D_8004CA88, D_8004CA94 };
    Sys411F4 *sys;
    s16 i;
    s16 j;
    s32 weight;
    u32 off;
    u32 def;
    double doff;
    double ddef;
    s32 offense;
    s32 stab;
    s32 mob;
    s32 speed;
    s32 radar;
    s32 sum;
    s32 fcs;
    u16 boost;
    s32 total;

    func_80071A18();
    for (j = 0; j < 11; j++) {
        if ((D_80039CA0 >> j) & 1) {
            D_8004AC3C.price += D_800BB140[j].price;
        }
    }
    func_8005D80C(1, D_8004BEBC, 16, 8, 6, 8);
    func_8005D80C(1, D_8004BEC0, 16, 8, 22, 8);
    func_8005D80C(1, D_8004CAB4, 16, 8, 38, 8);
    func_80085320(6, D_8017EF20, D_8004AC3C.ap);
    weight = 0;
    func_80085320(8, D_8017EF0C[0], D_8004AC3C.weight);
    func_80085320(8, D_8017EF0C[1], D_8004AC3C.price);
    func_8005D80C(1, D_8017EF20, 16, 72, 6, 8);
    func_8005D80C(1, D_8017EF0C[0], 16, 56, 22, 8);
    func_8005D80C(1, D_8017EF0C[1], 16, 56, 38, 8);
    func_8005CB94(D_80031BD4, 16, 4, 4);
    func_8005D80C(1, D_8004CABC, 6, 8, 24, 4);
    func_8005D80C(1, D_8004CACC, 6, 8, 36, 4);
    func_8005D80C(1, D_8004CADC, 6, 8, 48, 4);
    func_8005D80C(1, D_8004CAE8, 6, 8, 60, 4);
    func_8005D80C(1, D_8004CAF4, 6, 20, 72, 4);
    func_8005D80C(1, D_8004CB04, 6, 20, 84, 4);
    func_8005D80C(1, D_8004CB14, 6, 20, 96, 4);
    func_8005D80C(1, D_8004CB24, 6, 8, 108, 4);
    func_8005D80C(1, D_8004CB34, 6, 20, 120, 4);
    func_8005D80C(1, D_8004CB40, 6, 20, 132, 4);
    func_8005D80C(1, D_8004CB50, 6, 8, 144, 4);
    func_8005D80C(1, D_8004CB58, 6, 8, 162, 4);
    func_8009C964(0);
    func_80071A18();
    sys = &D_800411F4;
    if (D_80031A5B >= 8 && D_80031A5B != 255) {
        weight = D_800BB314[D_80031A5B - 8].weight;
    } else {
        if (D_80031A5D != 255) {
            weight += D_800BB3D4[D_80031A5D].weight;
        }
        if (D_80031A5C != 255) {
            weight += D_800BAB0C[D_80031A5C].weight;
        }
    }
    if (D_80031A5E != 255) {
        weight += D_800BB554[D_80031A5E].weight;
    }
    if (D_80031A5F != 255) {
        weight += D_800BB554[D_80031A5F].weight;
    }
    off = D_800B9222[D_80031A5A].off;
    off += D_800B9E22[D_80031A5B].off;
    off += D_800B9522[D_80031A63].off;
    off += D_800B947A[D_80031A58].off;
    def = D_800B9222[D_80031A5A].def;
    def += D_800B9E22[D_80031A5B].def;
    def += D_800B9522[D_80031A63].def;
    def += D_800B947A[D_80031A58].def;
    doff = off;
    ddef = def;
    if (D_80039CA0 & 0x80) {
        doff *= 1.15;
    }
    if (D_80039CA0 & 0x100) {
        ddef *= 1.15;
    }
    offense = doff + ddef;
    stab = D_800B9522[D_80031A63].stab * 10;
    mob = D_800B9522[D_80031A63].mob * 10;
    if (D_80031A63 >= 27) {
        speed = 9800 / D_8004AC3C.weight * 1000;
    } else {
        speed = D_800BABD8[D_80031A59].speed / D_8004AC3C.weight * 1000;
    }
    radar = (u16)(sys->radar / 10);
    fcs = (u16)(D_800BACF8[D_80031A61].b / 10);
    for (i = 0, sum = 0; i < 4; i++) {
        sum += D_800BACF8[D_80031A61].sub[i].x;
    }
    fcs += sum / 10;
    for (i = 0, sum = 0; i < 4; i++) {
        sum += D_800BACF8[D_80031A61].sub[i].y;
    }
    fcs += sum / 10;
    fcs += D_800BACF8[D_80031A61].a * 100;
    fcs -= D_800BACF8[D_80031A61].sub[0].z * 10;
    func_80085320(6, D_8017EF30[0], weight);
    func_80085320(6, D_8017EF30[1], offense);
    boost = D_800B9522[D_80031A63].boost;
    if (D_80039CA0 & 8) {
        boost += 800;
    }
    if (D_80039CA0 & 0x10) {
        boost += 800;
    }
    func_80085320(6, D_8017EF30[2], boost);
    func_80085320(6, D_8017EF30[3], stab + mob + speed);
    func_80085320(6, D_8017EF30[4], stab);
    func_80085320(6, D_8017EF30[5], mob);
    func_80085320(6, D_8017EF30[6], speed);
    func_80085320(6, D_8017EF30[7], radar + fcs);
    func_80085320(6, D_8017EF30[8], radar);
    func_80085320(6, D_8017EF30[9], fcs);
    total = weight + offense + boost + stab + mob + speed + radar + fcs;
    func_80085320(6, D_8017EF30[10], total);
    func_8005D80C(1, D_8017EF30[0], 6, 110, 24, 4);
    func_8005D80C(1, D_8017EF30[1], 6, 110, 36, 4);
    func_8005D80C(1, D_8017EF30[2], 6, 110, 48, 4);
    func_8005D80C(1, D_8017EF30[3], 6, 110, 60, 4);
    func_8005D80C(1, D_8017EF30[4], 6, 110, 72, 4);
    func_8005D80C(1, D_8017EF30[5], 6, 110, 84, 4);
    func_8005D80C(1, D_8017EF30[6], 6, 110, 96, 4);
    func_8005D80C(1, D_8017EF30[7], 6, 110, 108, 4);
    func_8005D80C(1, D_8017EF30[8], 6, 110, 120, 4);
    func_8005D80C(1, D_8017EF30[9], 6, 110, 132, 4);
    func_8005D80C(1, D_8017EF30[10], 6, 110, 144, 4);
    if (total >= 25000) {
        func_8005D80C(1, grade[4], 6, 74, 162, 4);
    } else if (total >= 20000) {
        func_8005D80C(1, grade[3], 6, 74, 162, 4);
    } else if (total >= 15000) {
        func_8005D80C(1, grade[2], 6, 74, 162, 4);
    } else if (total >= 10000) {
        func_8005D80C(1, grade[1], 6, 74, 162, 4);
    } else {
        func_8005D80C(1, grade[0], 6, 74, 162, 4);
    }
    o->update = func_8008630C;
}
