//
// Created by Seth Kingsley on 8/30/18.
//

#include <assert.h>
#include "clip.h"
#include "draw.h"

bool
clip_point(const struct device_vertex *p)
{
	return (p->coord.x >= -p->coord.w && p->coord.x <= p->coord.w &&
	        p->coord.y >= -p->coord.w && p->coord.y <= p->coord.w &&
	        p->coord.z >= -p->coord.w && p->coord.z <= p->coord.w);
}
