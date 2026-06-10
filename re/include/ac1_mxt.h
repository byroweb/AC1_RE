/* ac1_mxt.h — behavioral reference model of the Armored Core 1 (SLUS-01323)
 * "MXT" container system: the ".T" archives, the FDAT overlay loader, and the
 * trailing-word checksum (game routine FUN_80015b24). See docs/MXT_LOADER.md.
 *
 * This is NOT a byte-matching decompilation. It is a clean, portable C99
 * re-implementation of the *behaviour*, written to be readable and mutable for
 * tooling and mods. The game's real addresses are recorded as constants so the
 * model stays anchored to the binary.
 *
 * Anchors (resident exe, Ghidra DB):
 *   FUN_80016678  mxt_register(mxtid, path)         registry build
 *   FUN_800165e4  mxt_load_entry(mxtid, entry, dst) code-overlay loader
 *   FUN_80015b24  mxt_verify(buf, nSectors)         checksum (this file)
 */
#ifndef AC1_MXT_H
#define AC1_MXT_H

#include <stddef.h>
#include <stdint.h>

#define AC1_MXT_SECTOR        2048u           /* CD user-data bytes per sector  */
#define AC1_MXT_CHECKSUM_SEED 0x12345678u     /* FUN_80015b24 seed              */
#define AC1_OVL_LOAD_BASE     0x8004ada0u      /* PTR_DAT_800112c8: overlay base */
#define AC1_OVL_MAGIC         "ENERGY\0"      /* 8-byte overlay signature       */

/* TOC convention of a ".T" archive (sector 0 = uint16 table). */
typedef enum {
    AC1_TOC_AUTO = 0,    /* detect */
    AC1_TOC_OFFSET_FIRST, /* u16[0] = first entry's start sector (==1)        */
    AC1_TOC_COUNT_FIRST   /* u16[0] = entry count, offsets start at u16[1]    */
} ac1_toc_kind;

/* A parsed MXT archive (the de-sectored flat ".T" image, 2048 B/sector). */
typedef struct {
    const uint8_t *data;     /* whole container image                         */
    size_t         size;     /* bytes (multiple of AC1_MXT_SECTOR)            */
    uint32_t       sectors;  /* size / 2048                                   */
    ac1_toc_kind   kind;     /* resolved convention                          */
    uint32_t       count;    /* number of entries                            */
    /* offset[i] = start sector of entry i; offset[count] = end sentinel.     */
    uint16_t       offset[1025];
} ac1_mxt;

/* Overlay image header (first 16 bytes of a code overlay entry). */
typedef struct {
    uint32_t flags;        /* +0x00, observed 4                              */
    uint32_t entry_fn;     /* +0x04, main-entry fn ptr (load-base relative)  */
    char     magic[8];     /* +0x08, "ENERGY\0\0"                            */
} ac1_overlay_header;

/* Parse the TOC of a flat ".T" image. Returns 0 on success, -1 on bad input. */
int ac1_mxt_open(ac1_mxt *m, const uint8_t *data, size_t size, ac1_toc_kind hint);

/* Locate entry `i`: writes byte offset + byte length into *off / *len.
 * Returns 0 on success, -1 if i is out of range. Zero-length entries are ok. */
int ac1_mxt_entry(const ac1_mxt *m, uint32_t i, size_t *off, size_t *len);

/* The game checksum (FUN_80015b24): seed + sum of every 32-bit word except the
 * last; the last word is the stored checksum. `len` must be a sector multiple.
 * Returns the *computed* checksum (compare against the stored last word). */
uint32_t ac1_mxt_checksum(const void *entry, size_t len);

/* 1 if the entry's stored trailing word matches its computed checksum. */
int ac1_mxt_verify(const void *entry, size_t len);

/* Rewrite the trailing word so the entry validates (use after editing an entry
 * in a mutable buffer — the whole point of modding an overlay). */
void ac1_mxt_fix_checksum(void *entry, size_t len);

/* Read the 16-byte overlay header from an entry payload. */
void ac1_overlay_read_header(const void *entry, ac1_overlay_header *h);

#endif /* AC1_MXT_H */
