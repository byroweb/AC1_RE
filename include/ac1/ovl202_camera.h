/* include/ac1/ovl202_camera.h - overlay 202 view camera state at 0x801987D0
 * (func_80083038 per-frame update, func_80082E7C eye/target, func_80082D04 easing).
 * Include after "ac1/ac1.h". */
#ifndef AC1_OVL202_CAMERA_H
#define AC1_OVL202_CAMERA_H

typedef struct {                /* 0x801987D0: view camera state */
    u8 snap;                    /* +0x00 1 = jump, no easing, next frame */
    u8 eye_a;                   /* +0x01 passed to func_800745D0 (cockpit eye) */
    u8 eye_b;                   /* +0x02 */
    u8 pad03;
    u8 *parts;                  /* +0x04 part list for func_80055E20 */
    u16 mode;                   /* +0x08 bit 0 = eye fixed */
    u16 zoom;                   /* +0x0A 0x801987DA eye-target distance, 4096 = 1.0 */
    s16 height;                 /* +0x0C 0x801987DC */
    u16 ease;                   /* +0x0E eye easing shift */
    u8 pad10[0x0C];
    s16 eye[3];                 /* +0x1C 0x801987EC */
    u8 pad22[2];
    s16 at[3];                  /* +0x24 0x801987F4 */
    u8 pad2a[2];
    s16 ox;                     /* +0x2C 0x801987FC eased view offset (pitch) */
    s16 oy;                     /* +0x2E 0x801987FE (yaw) */
    u8 pad30[4];
    s32 frames;                 /* +0x34 0x80198804 */
} CamParm;

extern CamParm D_801987D0;

#endif
