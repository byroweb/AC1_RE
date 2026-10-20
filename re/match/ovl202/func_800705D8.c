#include "ac1/ac1.h"
/* ovl202 func_800705D8 - nearest-hit probe against the world structs'
 * collision meshes.  Every candidate world struct (bitset filled by
 * func_8006E0C0) is visited; small meshes are tested polygon by polygon,
 * meshes with 16+ polygons only in the buckets func_80070A58 selects.  The
 * hit nearest to the probe start (Manhattan distance) wins; its point is
 * fixed up by func_8006FA30, returned in out relative to the world struct's
 * origin, and the polygon / world struct go to D_801E8B74. */

extern void func_8006E0C0(void *arg0, CollScratch *scr);
extern s32 func_8006FB6C(void *bits, ...);
extern void func_8006FD6C(CollScratch *scr, void *arg0);
extern u32 func_80070A58(CollHdr *hdr, s32 *p, s32 *arg2);
extern s32 func_80070168(void *verts, CollPoly *poly, s32 *p, SVECTOR *hit);
extern void func_8006FA30(SVECTOR *from, SVECTOR *to, s32 arg2);
extern u32 D_801E8B70;
extern CollMesh D_80091228[];
extern CollHit D_801E8B74;

CollHit *func_800705D8(void *arg0, SVECTOR *out, s32 flags)
{
    CollScratch *scr;
    s32 dist;

    scr = CSCR;

    func_8006E0C0(arg0, scr);
    scr->best_dist = 0x7FFFFFFF;
    scr->best_poly = 0;
    scr->meshes = D_80091228;
    for (scr->idx = func_8006FB6C(scr, D_801E8B70); scr->idx != -1; scr->idx = func_8006FB6C(0)) {
        scr->ws = &g_world_structs[scr->idx];
        func_8006FD6C(scr, arg0);
        scr->mesh = &scr->meshes[scr->ws->mesh];
        scr->hdr = scr->mesh->hdr;
        if (scr->hdr->n < 16) {
            scr->d[0] -= scr->p[0];
            scr->d[1] -= scr->p[1];
            scr->d[2] -= scr->p[2];
            scr->cur = (CollPoly *)(scr->hdr + 1);
            scr->end = scr->group_end = scr->cur + scr->hdr->n;
            goto test;
        }
        scr->mask = func_80070A58(scr->hdr, scr->p, scr->unk44);
        if (scr->mask == 0) {
            continue;
        }
        scr->d[0] -= scr->p[0];
        scr->d[1] -= scr->p[1];
        scr->d[2] -= scr->p[2];
        scr->cur = (CollPoly *)(scr->hdr + 1);
        scr->end = scr->cur + scr->hdr->n;
        scr->group = (scr->hdr->n >> 5) + 1;
        do {
            if (scr->mask & 1) {
                scr->group_end = scr->cur + scr->group;
                if (scr->end < scr->group_end) {
                    scr->group_end = scr->end;
                }
            test:
                do {
                    if ((scr->cur->v01.h.hi & 7 & flags) == 0
                        && func_80070168(scr->mesh->verts, scr->cur, scr->p, &scr->hit)) {
                        dist = ((scr->p[0] < scr->hit.vx) ? scr->hit.vx - scr->p[0] : scr->p[0] - scr->hit.vx)
                             + ((scr->p[1] < scr->hit.vy) ? scr->hit.vy - scr->p[1] : scr->p[1] - scr->hit.vy)
                             + ((scr->p[2] < scr->hit.vz) ? scr->hit.vz - scr->p[2] : scr->p[2] - scr->hit.vz);
                        if (dist < scr->best_dist) {
                            scr->best_dist = dist;
                            scr->best_hit = scr->hit;
                            scr->best_ws = scr->ws;
                            scr->best_poly = scr->cur;
                            scr->best_mesh = scr->mesh;
                        }
                    }
                    scr->cur++;
                } while (scr->cur < scr->group_end);
            } else {
                scr->cur += scr->group;
            }
            scr->mask >>= 1;
        } while (scr->cur < scr->end);
    }
    if (scr->best_poly != 0) {
        func_8006FA30(&scr->best_hit, &scr->hit, scr->best_ws->unk16);
        out->vx = scr->best_ws->org[0] + (scr->hit.vx >> 2);
        out->vy = scr->best_ws->org[1] + (scr->hit.vy >> 2);
        out->vz = scr->best_ws->org[2] + (scr->hit.vz >> 2);
        D_801E8B74.poly = scr->best_poly;
        D_801E8B74.ws = scr->best_ws;
        return &D_801E8B74;
    }
    return 0;
}
