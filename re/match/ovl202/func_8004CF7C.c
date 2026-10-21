#include "ac1/ac1.h"

typedef struct {
    u8 b[8];
} Pair8;

typedef struct {
    Pair8 head;
    u8 rest[1];
} Src8004CF7C;

extern Pair8 D_80041B90[];
extern void func_80014B74(void *a, void *src, void *dst);

void func_8004CF7C(Src8004CF7C *s)
{
    D_80041B90[0] = s->head;
    func_80014B74(s, s->rest, &D_80041B90[1]);
}
