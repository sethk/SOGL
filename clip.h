//
// Created by Seth Kingsley on 8/29/18.
//

#ifndef SOGL_CLIP_H
#define SOGL_CLIP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct device_vertex;

bool clip_point(const struct device_vertex *p);
bool clip_line(struct device_vertex *p1, struct device_vertex *p2);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //SOGL_CLIP_H
