#include "ac1/ac1.h"

typedef struct {
    s8 count;
    s8 rows;
    u8 pad2[7];
    volatile u8 b9;             /* +0x009 */
    u8 padA[0x2F0 - 0xA];
    char text[1];               /* +0x2F0 */
} NameList;

typedef struct {
    u8 pad0[0x1944];
    NameList *list;             /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;

void func_800887B8(char *s)
{
    char *dst;
    char *p;

    dst = D_801F3750.list->text;
    p = s;
    while (*p != '>') {
        if (*p == '/') break;
        /* two identical arms: retail register allocation (permuter find) */
        if (s) {
            *dst++ = *p++;
        } else {
            *dst++ = *p++;
        }
    }
    dst[0] = *p;
    dst[1] = 0;
    free2(s);
    D_801F3750.list->b9 = 0;
}
