#include "ac1/ac1.h"

typedef struct {
    u8 pad[4];
    WorldStruct *ws;    /* +0x04 */
    u8 pad08[6];
    u16 h0e;            /* +0x0E */
} FloorHit;

typedef struct {
    u8 pad[0x2C];
    s16 floor;          /* +0x2C */
    s16 ceil;           /* +0x2E */
    s16 ws;             /* +0x30 */
} AcFloor;

extern s32 D_801A5DC8;
extern FloorHit *func_80075030(AcFloor *e, s32 a);
extern FloorHit *func_8006D4BC(s32 a);

void func_80078598(AcFloor *e)
{
    FloorHit *h;

    D_801A5DC8 = 1;
    h = func_80075030(e, 4096);
    if (h != 0) {
        e->floor = h->h0e;
        e->ws = h->ws - g_world_structs;
    } else {
        e->floor = 0x7FFF;
        e->ws = -1;
    }
    h = func_8006D4BC(1);
    if (h != 0) {
        e->ceil = h->h0e;
    } else {
        e->ceil = 0x7FFF;
    }
    D_801A5DC8 = 0;
}
