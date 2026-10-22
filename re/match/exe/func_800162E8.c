#include "ac1/ac1.h"

typedef struct {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
} CdlLOC;
typedef struct {
    CdlLOC pos;
    u32 size;
    char name[16];
} CdlFILE;
extern CdlFILE *CdSearchFile(CdlFILE *fp, char *name);

void func_800162E8(CdlLOC *loc, char *name)
{
    CdlFILE fp;

    CdSearchFile(&fp, name);
    *loc = fp.pos;
}
