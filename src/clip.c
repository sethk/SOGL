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

struct device_vertex
clip_vertex_lerp(const struct device_vertex *p1, const struct device_vertex *p2, scalar_t t)
{
	assert(t >= 0 && t <= 1);
	if (t == 0)
		return *p1;
	else if (t == 1)
		return *p2;
	else
	{
		struct device_vertex dv;
		dv.coord = vector4_lerp(p1->coord, p2->coord, t);
		dv.color = vector4_lerp(p1->color, p2->color, t);
		return dv;
	}
}
