/* core/xa.c — C port of tools/audio/xa_adpcm.decode_stream_float, for AC1mod.
 *
 * tools/audio/xa_adpcm.py is the XA-ADPCM decoder of record; this is only its hot
 * loop, because the ADPCM reconstruction is a serial IIR (each sample depends on
 * the previous two, filter/shift change every 28-sample sound unit and the history
 * chains through the whole channel) that neither numpy nor pure Python run fast.
 * It must stay BIT-IDENTICAL to decode_unit_float / decode_sector_data_stereo_float:
 *   s = (double)(t << shift) + p1*K0[f]/64 + p2*K1[f]/64      (that order)
 *   out = clamp(floor(s + 0.5)),  history = the UNclamped s
 * so it is built with -ffp-contract=off (no FMA fusing). core/xa.py feeds it the
 * sound areas from xa_adpcm.stream_sound_areas and falls back to
 * decode_stream_float when no compiler is available.
 *
 * Per 128-byte group: 16-byte header, then 28x4 data bytes; byte 0x10+4*i+j holds
 * sample i of unit 2j (low nibble) and unit 2j+1 (high nibble). Unit params are at
 * header bytes 4..11 (copies of 0..3 and 8..11): hdr[4+2j] = unit 2j, hdr[5+2j] =
 * unit 2j+1. Mono: units 0..7 are consecutive in time. Stereo: even units L, odd R.
 */
#include <math.h>

static const double K0[4] = {0.0, 60.0 / 64.0, 115.0 / 64.0, 98.0 / 64.0};
static const double K1[4] = {0.0, 0.0, -52.0 / 64.0, -55.0 / 64.0};

static inline int sx4(int n) {        /* sign-extend low 4 bits */
    n &= 0xF;
    return n >= 8 ? n - 16 : n;
}

static inline short out16(double s) {
    double v = floor(s + 0.5);
    if (v > 32767.0) return 32767;
    if (v < -32768.0) return -32768;
    return (short)v;
}

/* One 28-sample unit: nibble column `col` (0..3), `hi` selects the high nibble.
 * Writes sample k to o[k * step]; carries (p1, p2). */
static inline void unit(const unsigned char *dat, int col, int hi, int param,
                        double *p1, double *p2, short *o, int step)
{
    int shift = 12 - (param & 0x0F);
    if (shift < 0) shift = 0;         /* reserved 13..15 (xa_adpcm.py raises) */
    int f = (param >> 4) & 3;
    double a = *p1, b = *p2;
    for (int k = 0; k < 28; ++k) {
        int byte = dat[col + k * 4];
        int t = sx4(hi ? byte >> 4 : byte);
        double s = (double)(t * (1 << shift)) + a * K0[f] + b * K1[f];
        o[k * step] = out16(s);
        b = a; a = s;
    }
    *p1 = a; *p2 = b;
}

/* Decode n_groups XA sound groups (128 bytes each) into int16 PCM.
 *   in        : n_groups*128 bytes of concatenated sound groups
 *   channels  : 1 (mono) or 2 (stereo)
 *   out       : caller-allocated n_groups*224 shorts (stereo: interleaved L,R)
 */
void xa_decode(const unsigned char *in, int n_groups, int channels, short *out)
{
    double l1 = 0, l2 = 0, r1 = 0, r2 = 0;

    for (int g = 0; g < n_groups; ++g) {
        const unsigned char *hdr = in + g * 128;
        const unsigned char *dat = hdr + 16;
        short *o = out + (long)g * 224;

        for (int j = 0; j < 4; ++j) {
            if (channels == 2) {
                unit(dat, j, 0, hdr[4 + j * 2], &l1, &l2, o + j * 56, 2);
                unit(dat, j, 1, hdr[5 + j * 2], &r1, &r2, o + j * 56 + 1, 2);
            } else {
                unit(dat, j, 0, hdr[4 + j * 2], &l1, &l2, o + j * 56, 1);
                unit(dat, j, 1, hdr[5 + j * 2], &l1, &l2, o + j * 56 + 28, 1);
            }
        }
    }
}
