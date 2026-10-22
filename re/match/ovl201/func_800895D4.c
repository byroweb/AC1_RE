#include "ac1/ac1.h"

/* part tables: only the u16 weight / load column is touched, so each table is
 * declared at the address of that column (row stride = sizeof) */
typedef struct { u16 v; u8 pad[58]; } Row60;
typedef struct { u16 v; u8 pad[54]; } Row56;
typedef struct { u16 v; u8 pad[50]; } Row52;
typedef struct { u16 v; u8 pad[70]; } Row72;
typedef struct { u16 v; u8 pad[94]; } Row96;
typedef struct { u16 v; u8 pad[42]; } Row44;
typedef struct { u16 v; u8 pad[46]; } Row48;

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

extern Row60 D_800B921E[];
extern Row56 D_800B9476[];
extern Row52 D_800B9E1E[];
extern Row72 D_800B951E[];
extern Row96 D_800BACF6[];
extern Row52 D_800BA41E[];
extern Row52 D_800BAB06[];
extern Row44 D_800BA15E[];
extern Row48 D_800BABD6[];
extern Row48 D_800BAFF6[];

s32 func_800895D4(void)
{
    s32 w;

    w = D_800B921E[D_80031A5A].v;
    w += D_800B9476[D_80031A58].v;
    w += D_800B9E1E[D_80031A5B].v;
    w += D_800B951E[D_80031A63].v;
    w += D_800BACF6[D_80031A61].v;
    if (D_80031A5E != 255) w += D_800BA41E[D_80031A5E].v;
    if (D_80031A5F != 255) w += D_800BA41E[D_80031A5F].v;
    if (D_80031A5C != 255) w += D_800BAB06[D_80031A5C].v;
    if (D_80031A5D != 255) w += D_800BA15E[D_80031A5D].v;
    if (D_80031A59 != 255) w += D_800BABD6[D_80031A59].v;
    return D_800BAFF6[D_80031A60].v < w;
}
