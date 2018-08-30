//
// Created by Seth Kingsley on 8/29/18.
//

#include <assert.h>
#include "clip.h"
#include "draw.h"
#include "vector.h"

static bool
clip_unit_plane(struct vector3 *c1, const struct vector3 *c2, const struct vector3 *pnorm, scalar_t *tp)
{
	struct vector3 pc1 = vector3_sub(*pnorm, *c1);
	scalar_t pc1_dot_pnorm = vector3_dot(pc1, *pnorm);
	if (pc1_dot_pnorm < 0)
	{
		struct vector3 c1_c2 = vector3_sub(*c2, *c1);
		scalar_t v_dot_n = vector3_dot(c1_c2, *pnorm);
		if (v_dot_n >= 0)
			// c1 is outside and line points away from or parallel to plane
			return false;

		scalar_t t = pc1_dot_pnorm / v_dot_n;
		if (t < 1)
		{
			*tp = t;
			*c1 = vector3_lerp(*c1, *c2, *tp);
		}
		else
			// Both c1 and c2 are outside; should have been trivially rejected
			return false;
	}
	return true;
}

bool
clip_line(struct device_vertex *p1, struct device_vertex *p2)
{
	struct vector3 c1 = vector4_project(p1->coord);
	struct vector3 c2 = vector4_project(p2->coord);
	scalar_t t1 = 0, t2 = 0;

	static const struct vector3 top_plane = {.v = {0, 1, 0}},
			left_plane = {.v = {-1, 0, 0}},
			bottom_plane = {.v = {0, -1, 0}},
			right_plane = {.v = {1, 0, 0}},
			back_plane = {.v = {0, 0, -1}},
			front_plane = {.v = {0, 0, 1}};

	if (!clip_unit_plane(&c1, &c2, &left_plane, &t1) ||
			!clip_unit_plane(&c2, &c1, &left_plane, &t2) ||
			!clip_unit_plane(&c1, &c2, &right_plane, &t1) ||
			!clip_unit_plane(&c2, &c1, &right_plane, &t2) ||
			!clip_unit_plane(&c1, &c2, &top_plane, &t1) ||
			!clip_unit_plane(&c2, &c1, &top_plane, &t2) ||
			!clip_unit_plane(&c1, &c2, &bottom_plane, &t1) ||
			!clip_unit_plane(&c2, &c1, &bottom_plane, &t2) ||
			!clip_unit_plane(&c1, &c2, &back_plane, &t1) ||
			!clip_unit_plane(&c2, &c1, &back_plane, &t2) ||
			!clip_unit_plane(&c1, &c2, &front_plane, &t1) ||
			!clip_unit_plane(&c2, &c1, &front_plane, &t2))
		return false;

	if (t1 != 0)
	{
		p1->coord.x = c1.x;
		p1->coord.y = c1.y;
		p1->coord.z = c1.z;
		p1->coord.w = 1;
		p1->color = vector4_lerp(p1->color, p2->color, t1);
	}

	if (t2 != 0)
	{
		p2->coord.x = c2.x;
		p2->coord.y = c2.y;
		p2->coord.z = c2.z;
		p2->coord.w = 1;
		p2->color = vector4_lerp(p2->color, p1->color, t2);
	}

	return true;
}

