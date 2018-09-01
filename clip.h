//
// Created by Seth Kingsley on 8/29/18.
//

#ifndef SOGL_CLIP_H
#define SOGL_CLIP_H

#include <sys/types.h>
#include <stdbool.h>
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct device_vertex;

bool clip_point(const struct device_vertex *p);
bool clip_line(const struct device_vertex *p1, const struct device_vertex *p2,
               struct device_vertex *clip_p1, struct device_vertex *clip_p2);
u_int clip_polygon(const struct device_vertex *verts, u_int num_verts, struct device_vertex *clipped_verts);

struct device_vertex clip_vertex_lerp(const struct device_vertex *p1, const struct device_vertex *p2, scalar_t t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //SOGL_CLIP_H
