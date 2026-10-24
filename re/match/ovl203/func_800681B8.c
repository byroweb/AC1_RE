/* ovl202 0x80068D18  size 3396  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-23 */
#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {                /* 0x10 projected vertex with clip codes (0x801A6A48) */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    u8 clip;                    /* +0x06 */
    u8 near;                    /* +0x07 */
    u8 col[3];                  /* +0x08 */
    u8 lit;                     /* +0x0B */
    s32 dp;                     /* +0x0C */
} CVtx;

typedef struct {                /* 0x1C model part */
    SVECTOR *verts;             /* +0x00 */
    u16 n_verts;                /* +0x04 */
    u16 pad06;
    u8 *normals;                /* +0x08 */
    u32 pad0c;
    u32 *prims;                 /* +0x10 */
    u16 n_prims;                /* +0x14 */
    u16 pad16;
    u32 pad18;
} MPart;

typedef struct {
    u32 *ot;                    /* +0x000 0x801A5F28 */
    u8 pad004[0x66C];
    CVtx vtx[1];                /* +0xB20 0x801A6A48 */
} OtCVtx;

typedef struct {                /* deferred (near) primitive */
    u32 *rec;
    u32 hdr;
} PList;

typedef struct {
    s32 nclip;                  /* +0x000 */
    u8 *prim;                   /* +0x004 */
    u8 *end;                    /* +0x008 */
    u8 pad00c[0x344];
    s8 zbias;                   /* +0x350 header word */
    u8 size;                    /* +0x351 */
    u8 sub;                     /* +0x352 subdivision levels: near 1 low 3 bits, near 2 high */
    u8 code;                    /* +0x353 */
    MPart *part;                /* +0x354 */
    u8 *normals;                /* +0x358 */
    SVECTOR *verts;             /* +0x35C */
    u8 pad360[8];
    s32 count;                  /* +0x368 */
    u32 *ot;                    /* +0x36C */
    u8 pad370[4];
    u8 *ctab;                   /* +0x374 */
    s32 sh3;                    /* +0x378 */
    s32 sh4;                    /* +0x37C */
    PList *l0;                  /* +0x380 quads, flat */
    PList *l1;                  /* +0x384 triangles, flat */
    PList *l2;                  /* +0x388 quads, lit */
    PList *l3;                  /* +0x38C triangles, lit */
    u8 r, g, b;                 /* +0x390 */
} CScr;

#define CSCR ((CScr *)0x1F800000)

#define VTX(o) ((CVtx *)((u8 *)vb + (o)))
#define R16(o) (*(u16 *)((u8 *)rec + (o)))
#define R32(o) (*(u32 *)((u8 *)rec + (o)))
#define NRM(o) (scr->normals + R16(o))
#define PADC(o) (scr->ctab + ((SVECTOR *)((u8 *)scr->verts + (R16(o) >> 1)))->pad)

extern OtCVtx D_801A5F28;
extern u8 *D_801A5F30;
extern DrawCtx D_801A5F24;
extern SVECTOR *D_801A6544;
extern s32 D_801987C8;
extern u8 D_80090A28[][128];
extern void func_80068A44(s32 n);
extern s32 func_80068CB8(CScr *s, s32 a, s32 b, s32 c);
extern void func_80068CE0(u8 *nrm, u8 *col);
extern void func_80068D00(u8 *col);
extern void func_80069F04(PList *l);
extern void func_8006A7EC(PList *l);
extern void func_8006B0BC(PList *l);
extern void func_8006BC30(PList *l);

#define LIST(l)                                 \
    scr->l->rec = rec;                          \
    scr->sub = s;                               \
    scr->l->hdr = *(u32 *)&scr->zbias;          \
    scr->l++;

#define NEAR(lbl, cull)                         \
    if (n < 2) {                                \
        s = scr->sub & 7;                       \
        if (s == 0) {                           \
            goto lbl;                           \
        }                                       \
        if (cull) {                             \
            break;                              \
        }                                       \
    } else {                                    \
        s = scr->sub >> 3;                      \
        if (s == 0) {                           \
            goto lbl;                           \
        }                                       \
    }

/* world-structure model part with near-plane handling (cf. func_80067EBC
 * and func_8006308C): far polygons drawn flat (NCCS) or with vertex
 * colours from table D_80090A28[nib], near ones queued for the subdividers */
void func_800681B8(u16 idx, s32 nib)
{
  PList b0[256];
  PList b1[256];
  PList b2[256];
  PList b3[256];
  CScr *scr;
  CVtx *vb;
  u32 *rec;
  CVtx *a;
  CVtx *b;
  int new_var;
  CVtx *c;
  CVtx *d;
  POLY_GT4 *p;
  u32 n;
  s32 oz;
  s32 s;
  scr = (CScr *) 0x1F800000;
  ((CScr *) 0x1F800000)->r = (((CScr *) 0x1F800000)->g = (((CScr *) 0x1F800000)->b = 64));
  ((CScr *) 0x1F800000)->l0 = b0;
  ((CScr *) 0x1F800000)->l1 = b1;
  ((CScr *) 0x1F800000)->l2 = b2;
  ((CScr *) 0x1F800000)->l3 = b3;
  ((CScr *) 0x1F800000)->sh3 = D_801987C8 + 15;
  ((CScr *) 0x1F800000)->sh4 = D_801987C8 + 3;
  ((CScr *) 0x1F800000)->ot = D_801A5F28.ot + 120;
  ((CScr *) 0x1F800000)->part = (MPart *) ((D_801A5F30 + ((idx * 32) + 12)) - (idx * 4));
  vb = D_801A5F28.vtx;
  func_80068A44(((CScr *) 0x1F800000)->part->n_verts);
  ((CScr *) 0x1F800000)->normals = ((CScr *) 0x1F800000)->part->normals;
  ((CScr *) 0x1F800000)->verts = D_801A6544;
  ((CScr *) 0x1F800000)->ctab = D_80090A28[nib];
  rec = ((CScr *) 0x1F800000)->part->prims;
  ((CScr *) 0x1F800000)->count = ((CScr *) 0x1F800000)->part->n_prims;
  ((CScr *) 0x1F800000)->prim = D_801A5F24.buf->cur;
  ((CScr *) 0x1F800000)->end = D_801A5F24.buf->end;
  while (scr->count--)
  {
    *((u32 *) (&scr->zbias)) = *(rec++);
    switch (scr->code & 0xFD)
    {
      case 0x24:
        a = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0xE))));
        b = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x10))));
        c = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x12))));
        if (c->clip & (a->clip & b->clip))
      {
        break;
      }
        n = c->near | (a->near | b->near);
        if (n == 0)
      {
        tri:
        if ((c->clip | (a->clip | b->clip)) == 31)
        {
          break;
        }

        if (func_80068CB8(scr, a->sxy, b->sxy, c->sxy) <= 0)
        {
          break;
        }
        asm volatile("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(scr->normals + (*((u16 *) (((u8 *) rec) + 0xC)))));
        asm volatile("lwc2 $6, 0(%0)" : : "r"(&scr->r));
        asm volatile("nop\n\tnop\n\t.word 0x4B08041B");
        p = (POLY_GT4 *) scr->prim;
        scr->prim = ((u8 *) p) + (sizeof(POLY_FT3));
        *((s32 *) (&((POLY_FT3 *) p)->x0)) = a->sxy;
        *((s32 *) (&((POLY_FT3 *) p)->x1)) = b->sxy;
        *((s32 *) (&((POLY_FT3 *) p)->x2)) = c->sxy;
        *((u32 *) (&((POLY_FT3 *) p)->u0)) = *((u32 *) (((u8 *) rec) + 0));
        *((u32 *) (&((POLY_FT3 *) p)->u1)) = *((u32 *) (((u8 *) rec) + 4));
        *((u16 *) (&((POLY_FT3 *) p)->u2)) = *((u16 *) (((u8 *) rec) + 8));
        ((P_TAG *) p)->len = (u8) 7;
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r0) : "memory");
        ((P_TAG *) p)->code = (u8) scr->code;
        oz = ((((a->otz + b->otz) + c->otz) * 0x1555) >> scr->sh3) + (scr->zbias << 2);
        ((P_TAG *) p)->addr = (u32) ((u32) ((P_TAG *) (scr->ot + oz))->addr), ((P_TAG *) (scr->ot + oz))->addr = (u32) p;
        break;
      }
        if (n < 2)
      {
        s = scr->sub & 7;
        if (s == 0)
        {
          goto tri;
        }
        if (func_80068CB8(scr, a->sxy, b->sxy, c->sxy) <= 0)
        {
          break;
        }
      }
      else
      {
        s = scr->sub >> 3;
        if (s == 0)
        {
          goto tri;
        }
      }
        scr->l1->rec = rec;
        scr->sub = s;
        scr->l1->hdr = *((u32 *) (&scr->zbias));
        scr->l1++;
        break;

      case 0x2C:
        a = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x12))));
        b = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x14))));
        c = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x16))));
        d = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x18))));
        if (d->clip & (c->clip & (a->clip & b->clip)))
      {
        break;
      }
        n = d->near | (c->near | (a->near | b->near));
        if (n == 0)
      {
        quad:
        if ((d->clip | (c->clip | (a->clip | b->clip))) == 31)
        {
          break;
        }

        if (func_80068CB8(scr, a->sxy, b->sxy, c->sxy) <= 0)
        {
          break;
        }
        asm volatile("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(scr->normals + (*((u16 *) (((u8 *) rec) + 0x10)))));
        asm volatile("lwc2 $6, 0(%0)" : : "r"(&scr->r));
        asm volatile("nop\n\tnop\n\t.word 0x4B08041B");
        p = (POLY_GT4 *) scr->prim;
        new_var = sizeof(POLY_FT4);
        scr->prim = ((u8 *) p) + new_var;
        *((s32 *) (&((POLY_FT4 *) p)->x0)) = a->sxy;
        *((s32 *) (&((POLY_FT4 *) p)->x1)) = b->sxy;
        *((s32 *) (&((POLY_FT4 *) p)->x2)) = c->sxy;
        *((s32 *) (&((POLY_FT4 *) p)->x3)) = d->sxy;
        *((u32 *) (&((POLY_FT4 *) p)->u0)) = *((u32 *) (((u8 *) rec) + 0));
        *((u32 *) (&((POLY_FT4 *) p)->u1)) = *((u32 *) (((u8 *) rec) + 4));
        *((u16 *) (&((POLY_FT4 *) p)->u2)) = *((u16 *) (((u8 *) rec) + 8));
        *((u16 *) (&((POLY_FT4 *) p)->u3)) = *((u16 *) (((u8 *) rec) + 0xC));
        ((P_TAG *) p)->len = (u8) 9;
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r0) : "memory");
        ((P_TAG *) p)->code = (u8) scr->code;
        oz = ((((a->otz + b->otz) + c->otz) + d->otz) >> scr->sh4) + (scr->zbias << 2);
        ((P_TAG *) p)->addr = (u32) ((u32) ((P_TAG *) (scr->ot + oz))->addr), ((P_TAG *) (scr->ot + oz))->addr = (u32) p;
        break;
      }
        if (n < 2)
      {
        s = scr->sub & 7;
        if (s == 0)
        {
          goto quad;
        }
        if (func_80068CB8(scr, a->sxy, b->sxy, c->sxy) <= 0)
        {
          break;
        }
      }
      else
      {
        s = scr->sub >> 3;
        if (s == 0)
        {
          goto quad;
        }
      }
        goto ql0;

      case 0x25:
        a = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0xE))));
        b = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x10))));
        c = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x12))));
        if (c->clip & (a->clip & b->clip))
      {
        break;
      }
        n = c->near | (a->near | b->near);
        if (n == 0)
      {
        gtri:
        if ((c->clip | (a->clip | b->clip)) == 31)
        {
          break;
        }

        if (func_80068CB8(scr, a->sxy, b->sxy, c->sxy) <= 0)
        {
          break;
        }
        func_80068CE0(scr->normals + (*((u16 *) (((u8 *) rec) + 0xC))), scr->ctab + ((SVECTOR *) (((u8 *) scr->verts) + ((*((u16 *) (((u8 *) rec) + 0xE))) >> 1)))->pad);
        p = (POLY_GT4 *) scr->prim;
        scr->prim = ((u8 *) p) + (sizeof(POLY_GT3));
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r0) : "memory");
        func_80068D00(scr->ctab + ((SVECTOR *) (((u8 *) scr->verts) + ((*((u16 *) (((u8 *) rec) + 0x10))) >> 1)))->pad);
        *((s32 *) (&p->x0)) = a->sxy;
        *((s32 *) (&p->x1)) = b->sxy;
        *((s32 *) (&p->x2)) = c->sxy;
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r1) : "memory");
        func_80068D00(scr->ctab + ((SVECTOR *) (((u8 *) scr->verts) + ((*((u16 *) (((u8 *) rec) + 0x12))) >> 1)))->pad);
        *((u32 *) (&p->u0)) = *((u32 *) (((u8 *) rec) + 0));
        *((u32 *) (&p->u1)) = *((u32 *) (((u8 *) rec) + 4));
        *((u16 *) (&p->u2)) = *((u16 *) (((u8 *) rec) + 8));
        ((P_TAG *) p)->len = (u8) 9;
        ((P_TAG *) p)->code = (u8) ((scr->code & 2) | 0x34);
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r2) : "memory");
        oz = ((((a->otz + b->otz) + c->otz) * 0x1555) >> scr->sh3) + (scr->zbias << 2);
        ((P_TAG *) p)->addr = (u32) ((u32) ((P_TAG *) (scr->ot + oz))->addr), ((P_TAG *) (scr->ot + oz))->addr = (u32) p;
        break;
      }
        if (n < 2)
      {
        s = scr->sub & 7;
        if (s == 0)
        {
          goto gtri;
        }
        if (func_80068CB8(scr, a->sxy, b->sxy, c->sxy) <= 0)
        {
          break;
        }
      }
      else
      {
        s = scr->sub >> 3;
        if (s == 0)
        {
          goto gtri;
        }
      }
        scr->l3->rec = rec;
        scr->sub = s;
        scr->l3->hdr = *((u32 *) (&scr->zbias));
        scr->l3++;
        break;

      case 0x2D:
        a = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x12))));
        b = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x14))));
        c = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x16))));
        d = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x18))));
        if (d->clip & (c->clip & (a->clip & b->clip)))
      {
        break;
      }
        n = d->near | (c->near | (a->near | b->near));
        if (n == 0)
      {
        gquad:
        if ((d->clip | (c->clip | (a->clip | b->clip))) == 31)
        {
          break;
        }

        if (func_80068CB8(scr, a->sxy, b->sxy, c->sxy) <= 0)
        {
          break;
        }
        func_80068CE0(scr->normals + (*((u16 *) (((u8 *) rec) + 0x10))), scr->ctab + ((SVECTOR *) (((u8 *) scr->verts) + ((*((u16 *) (((u8 *) rec) + 0x12))) >> 1)))->pad);
        p = (POLY_GT4 *) scr->prim;
        scr->prim = (u8 *) (p + 1);
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r0) : "memory");
        func_80068D00(scr->ctab + ((SVECTOR *) (((u8 *) scr->verts) + ((*((u16 *) (((u8 *) rec) + 0x14))) >> 1)))->pad);
        *((s32 *) (&p->x0)) = a->sxy;
        *((s32 *) (&p->x1)) = b->sxy;
        *((s32 *) (&p->x2)) = c->sxy;
        *((s32 *) (&p->x3)) = d->sxy;
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r1) : "memory");
        func_80068D00(scr->ctab + ((SVECTOR *) (((u8 *) scr->verts) + ((*((u16 *) (((u8 *) rec) + 0x16))) >> 1)))->pad);
        *((u32 *) (&p->u0)) = *((u32 *) (((u8 *) rec) + 0));
        *((u32 *) (&p->u1)) = *((u32 *) (((u8 *) rec) + 4));
        *((u16 *) (&p->u2)) = *((u16 *) (((u8 *) rec) + 8));
        *((u16 *) (&p->u3)) = *((u16 *) (((u8 *) rec) + 0xC));
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r2) : "memory");
        func_80068D00(scr->ctab + ((SVECTOR *) (((u8 *) scr->verts) + ((*((u16 *) (((u8 *) rec) + 0x18))) >> 1)))->pad);
        ((P_TAG *) p)->len = (u8) 12;
        ((P_TAG *) p)->code = (u8) ((scr->code & 2) | 0x3C);
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r3) : "memory");
        oz = ((((a->otz + b->otz) + c->otz) + d->otz) >> scr->sh4) + (scr->zbias << 2);
        ((P_TAG *) p)->addr = (u32) ((u32) ((P_TAG *) (scr->ot + oz))->addr), ((P_TAG *) (scr->ot + oz))->addr = (u32) p;
        break;
      }
        if (n < 2)
      {
        s = scr->sub & 7;
        if (s == 0)
        {
          goto gquad;
        }
        if (func_80068CB8(scr, a->sxy, b->sxy, c->sxy) <= 0)
        {
          break;
        }
      }
      else
      {
        s = scr->sub >> 3;
        if (s == 0)
        {
          goto gquad;
        }
      }
        scr->l2->rec = rec;
        scr->sub = s;
        scr->l2->hdr = *((u32 *) (&scr->zbias));
        scr->l2++;
        break;

      case 0x6C:
        a = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x12))));
        b = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x14))));
        c = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x16))));
        d = (CVtx *) (((u8 *) vb) + (*((u16 *) (((u8 *) rec) + 0x18))));
        if (d->clip & (c->clip & (a->clip & b->clip)))
      {
        break;
      }
        n = d->near | (c->near | (a->near | b->near));
        if (n == 0)
      {
        quad2:
        if ((d->clip | (c->clip | (a->clip | b->clip))) == 31)
        {
          break;
        }

        asm volatile("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(scr->normals + (*((u16 *) (((u8 *) rec) + 0x10)))));
        asm volatile("lwc2 $6, 0(%0)" : : "r"(&scr->r));
        asm volatile("nop\n\tnop\n\t.word 0x4B08041B");
        p = (POLY_GT4 *) scr->prim;
        scr->prim = ((u8 *) p) + new_var;
        *((s32 *) (&((POLY_FT4 *) p)->x0)) = a->sxy;
        *((s32 *) (&((POLY_FT4 *) p)->x1)) = b->sxy;
        *((s32 *) (&((POLY_FT4 *) p)->x2)) = c->sxy;
        *((s32 *) (&((POLY_FT4 *) p)->x3)) = d->sxy;
        *((u32 *) (&((POLY_FT4 *) p)->u0)) = *((u32 *) (((u8 *) rec) + 0));
        *((u32 *) (&((POLY_FT4 *) p)->u1)) = *((u32 *) (((u8 *) rec) + 4));
        *((u16 *) (&((POLY_FT4 *) p)->u2)) = *((u16 *) (((u8 *) rec) + 8));
        *((u16 *) (&((POLY_FT4 *) p)->u3)) = *((u16 *) (((u8 *) rec) + 0xC));
        ((P_TAG *) p)->len = (u8) 9;
        asm volatile("swc2 $22, 0(%0)" : : "r"(&p->r0) : "memory");
        ((P_TAG *) p)->code = (u8) (scr->code & 0xBF);
        oz = ((((a->otz + b->otz) + c->otz) + d->otz) >> scr->sh4) + (scr->zbias << 2);
        ((P_TAG *) p)->addr = (u32) ((u32) ((P_TAG *) (scr->ot + oz))->addr), ((P_TAG *) (scr->ot + oz))->addr = (u32) p;
        break;
      }
        if (n < 2)
      {
        s = scr->sub & 7;
      }
      else
      {
        s = scr->sub >> 3;
      }
        if (s == 0)
      {
        goto quad2;
      }
        ql0:
      scr->l0->rec = rec;

        scr->sub = s;
        scr->l0->hdr = *((u32 *) (&scr->zbias));
        scr->l0++;
        break;

    }

    rec += scr->size;
  }

  scr->sh3 -= 14;
  scr->l1->rec = 0;
  func_80069F04(b1);
  scr->l0->rec = 0;
  func_8006A7EC(b0);
  scr->l3->rec = 0;
  func_8006B0BC(b3);
  scr->l2->rec = 0;
  func_8006BC30(b2);
  D_801A5F24.buf->cur = scr->prim;
}
