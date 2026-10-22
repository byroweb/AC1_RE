#include "ac1/ac1.h"

/* part tables, declared from the weight column (+0x00 here) */
typedef struct { u16 w; u8 pad[58]; } Head60;
typedef struct { u16 w; u8 pad2[8]; u16 load; u8 padC[44]; } Core56;
typedef struct { u16 w; u8 pad[50]; } Row52;
typedef struct { u16 w; u8 pad2[8]; u16 load; u8 padC[60]; } Legs72;
typedef struct { u16 w; u8 pad[94]; } Row96;
typedef struct { u16 w; u8 pad[42]; } Row44;
typedef struct { u16 w; u8 pad[46]; } Row48;

extern u8 D_80031A58; /* core */
extern u8 D_80031A59; /* booster */
extern u8 D_80031A5A; /* head */
extern u8 D_80031A5B; /* arms */
extern u8 D_80031A5C;
extern u8 D_80031A5D;
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern u8 D_80031A60;
extern u8 D_80031A61; /* FCS */
extern u8 D_80031A63; /* legs */

extern Head60 D_800B921C[];
extern Core56 D_800B9474[];
extern Row52 D_800B9E1C[];
extern Legs72 D_800B951C[];
extern Row96 D_800BACF4[];
extern Row52 D_800BA41C[];
extern Row52 D_800BAB04[];
extern Row44 D_800BA15C[];
extern Row48 D_800BABD4[];
extern Row48 D_800BAFF4[];

s32 func_8008935C(void)
{
    u16 arms;
    s32 aw;
    s32 w;

    arms = D_800B9E1C[D_80031A5B].w;
    aw = arms;
    if (D_80031A5C != 255) aw += D_800BAB04[D_80031A5C].w;
    if (D_80031A5D != 255) aw += D_800BA15C[D_80031A5D].w;
    if (D_800B9474[D_80031A58].load < aw) return 1;
    w = D_800B921C[D_80031A5A].w;
    w += D_800B9474[D_80031A58].w;
    w = arms + w;
    w += D_800BACF4[D_80031A61].w;
    if (D_80031A5E != 255) w += D_800BA41C[D_80031A5E].w;
    if (D_80031A5F != 255) w += D_800BA41C[D_80031A5F].w;
    if (D_80031A5C != 255) w += D_800BAB04[D_80031A5C].w;
    if (D_80031A5D != 255) w += D_800BA15C[D_80031A5D].w;
    if (D_80031A59 != 255) w += D_800BABD4[D_80031A59].w;
    w += D_800BAFF4[D_80031A60].w;
    return D_800B951C[D_80031A63].load < w;
}
