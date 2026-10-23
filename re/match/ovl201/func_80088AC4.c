/* ovl201 0x80088AC4  size 800  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-22 */
#include "ac1/ac1.h"

typedef struct LetterLine {
    u8 text[0x50];              /* +0x00 control codes + text */
    s32 pos;                    /* +0x50 read position in the source */
    s32 unk54;                  /* +0x54 */
    s32 len;                    /* +0x58 write position in text */
    s32 unk5C;                  /* +0x5C */
} LetterLine;

typedef struct {
    s8 count;                   /* +0x000 */
    s8 rows;                    /* +0x001 */
    u8 b2;                      /* +0x002 */
    u8 b3;                      /* +0x003 */
    u8 pad4[4];
    s8 page;                    /* +0x008 */
    u8 pad9;
    volatile u8 loading;        /* +0x00A */
    u8 padB[0x50 - 0xB];
    UiObj *win[3];              /* +0x050 */
    LetterLine line[5];         /* +0x05C */
    u8 shown[3];                /* +0x23C */
} NameList;

typedef struct {
    u8 pad0[0x79];
    s8 cursor;                  /* +0x079 0x801F37C9 */
    u8 pad7A[0x84 - 0x7A];
    s8 order[64];               /* +0x084 0x801F37D4 */
    u8 padC4[0xC44 - 0xC4];
    struct { s16 v[16]; } tbl[64]; /* +0xC44 0x801F4394 */
    u8 pad1444[0x1944 - 0x1444];
    NameList *list;             /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;
extern s16 D_801A2318;
extern void func_800513F0(s32 a, u16 b, void *cb);
extern void func_80088940(void *p);
extern UiObj *func_8008B778(void);
extern UiObj *func_8008B918(void);
extern UiObj *func_8008BAB8(void);

void func_80088AC4(LetterLine *l, u8 *src, s32 more)
{
  u8 *s;
  u8 *d;
  s32 i;
  s32 j;
  s32 pos;
  s32 k;
  s32 n;
  int c;
  do
  {
    pos = l->pos;
    s = src;
    s = s + pos;
    c = *s;
    d = l->text + l->len;
    if (c == '/')
    {
      for (i = 4; i != (-1); i--)
      {
        D_801F3750.list->line[i].pos = 0xFFFF;
      }

      D_801F3750.list->b2 = 1;
      D_801F3750.list->b3 = 1;
      return;
    }
    if (c == '>')
    {
      for (j = 4; j != (-1); j--)
      {
        D_801F3750.list->line[j].pos = 0xFFFF;
      }

      D_801F3750.list->b2 = 1;
      return;
    }
    if (c == '%')
    {
      k = s[1] - '0';
      l->pos = pos + 2;
      if (D_801F3750.list->page >= k)
      {
        return;
      }
      k %= 3;
      if (((u32) k) >= 3)
      {
        return;
      }
      D_801F3750.list->page++;
      n = D_801F3750.tbl[D_801F3750.order[D_801F3750.cursor]].v[D_801F3750.list->page];
      if (n >= 0)
      {
        while (D_801F3750.list->loading != 0)
        {
        }

        D_801F3750.list->loading = k + 1;
        func_800513F0(19, n + 1, func_80088940);
      }
      if (D_801F3750.list->shown[k] == 0)
      {
        D_801F3750.list->shown[k] = 1;
        switch (k)
        {
          case 0:
            D_801F3750.list->win[0] = func_8008B778();
            break;

          case 1:
            D_801F3750.list->win[1] = func_8008B918();
            break;

          case 2:
            D_801F3750.list->win[2] = func_8008BAB8();
            break;

        }

      }
      return;
    }
    if (c == '\r')
    {
      more = 0;
      l->unk54 = pos + 2;
      l->pos = 0xFFFF;
      l->unk5C = 1;
    }
    else
    {
      if ((c == '@') || (c == '^'))
      {
        l->pos = pos + 1;
        *(d++) = *(s++);
      }
      l->pos++;
      *(d++) = *s;
    }
    l->len = d - l->text;
    *d = '>';
    d[1] = 0;
  }
  while (more);
  SsUtKeyOn(D_801A2318, 0, 9, 60, 0, 127, 127);
}
