/* include/ac1/gfxframe.h - overlay 202 double-buffered frame state.
 *
 * Not part of the ac1.h umbrella: include it after "ac1/ac1.h".
 * Layouts proven by func_8005715C (flip + ClearOTagR), func_800571F0
 * (PutDispEnv/PutDrawEnv/DrawOTag) and func_800572A8 (buffer setup).
 * DRAWENV / DISPENV are the public PsyQ libgpu layouts.
 */
#ifndef AC1_GFXFRAME_H
#define AC1_GFXFRAME_H

typedef struct DRAWENV {        /* 0x5C */
    RECT clip;
    s16 ofs[2];
    RECT tw;
    u16 tpage;
    u8 dtd;                     /* +0x16 */
    u8 dfe;
    u8 isbg;                    /* +0x18 */
    u8 r0, g0, b0;
    u32 dr_env[16];             /* +0x1C DR_ENV */
} DRAWENV;
AC1_ASSERT_SIZE(DRAWENV, 0x5C);

typedef struct DISPENV {        /* 0x14 */
    RECT disp;
    RECT screen;
    u8 isinter, isrgb24, pad0, pad1;
} DISPENV;
AC1_ASSERT_SIZE(DISPENV, 0x14);

typedef struct GfxFrame {       /* 0x80: one of the two display buffers */
    u32 *ot;                    /* +0x00 ordering table (0x2000 entries) */
    DRAWENV draw;               /* +0x04 */
    DISPENV disp;               /* +0x60 */
    PrimBuf prim;               /* +0x74 primitive buffer */
} GfxFrame;
AC1_ASSERT_SIZE(GfxFrame, 0x80);

typedef struct GfxFrameSet {    /* 0x801A5E18 */
    s32 flip;                   /* index of the frame being built */
    GfxFrame f[2];              /* 0x801A5E1C, 0x801A5E9C */
} GfxFrameSet;

typedef struct GfxState {       /* 0x801A5F1C */
    GfxFrame *frame;            /* frame being built */
    u8 *heap;                   /* +0x04 start of the primitive heap */
    DrawCtx ctx;                /* +0x08 = 0x801A5F24 */
} GfxState;

typedef struct GfxCounters {    /* 0x801AC810 */
    s32 rcnt;                   /* GetRCnt at the last flip */
    s32 a;                      /* +0x04 cleared every flip */
    s32 b;                      /* +0x08 cleared every flip */
} GfxCounters;

extern GfxFrameSet D_801A5E18;
extern GfxState D_801A5F1C;
extern GfxCounters D_801AC810;

#endif
