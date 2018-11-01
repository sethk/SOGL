//
// Created by Seth Kingsley on 8/29/18.
//

#include <assert.h>
#include <math.h>
#include <strings.h>
#include "clip.h"
#include "draw.h"
#include "vector.h"

struct line_seg
{
	struct vector3 start;
	struct vector3 dir;
	scalar_t start_t, end_t;
};

static const struct vector3 clip_planes[] =
		{
				{.v = {0, -1, 0}},
				{.v = {0, 1, 0}},
				{.v = {-1, 0, 0}},
				{.v = {1, 0, 0}},
				{.v = {0, 0, -1}},
				{.v = {0, 0, 1}}
		};

static struct line_seg
clip_make_line_seg(const struct vector3 *start, const struct vector3 *end)
{
	return (struct line_seg){.start = *start, .dir = vector3_sub(*end, *start), .start_t = 0, .end_t = 1};
}

static void
clip_line_seg_unit_plane(struct line_seg *seg, const struct vector3 *pnorm)
{
	scalar_t norm_dot_dir = vector3_dot(*pnorm, seg->dir);
	if (norm_dot_dir != 0) // Ignore lines parallel to edge for now
	{
		struct vector3 edge_to_start = vector3_sub(seg->start, *pnorm);
		scalar_t num = vector3_dot(*pnorm, edge_to_start);
		scalar_t denom = -norm_dot_dir;
		scalar_t t = num / denom;
		if (denom > 0)
			seg->start_t = fmax(seg->start_t, t);
		else
			seg->end_t = fmin(seg->end_t, t);
	}
}

static bool
clip_line_seg_unit_cube(struct line_seg *seg)
{
	for (u_int plane_index = 0; plane_index < sizeof(clip_planes) / sizeof(clip_planes[0]); ++plane_index)
		clip_line_seg_unit_plane(seg, &(clip_planes[plane_index]));

	if (seg->start_t > seg->end_t)
		return false;

	return true;
}

// Cyrus-Beck Parametric Line Clip: unoptimized version
bool
clip_line(const struct device_vertex *p1, const struct device_vertex *p2,
          struct device_vertex *clip_p1, struct device_vertex *clip_p2)
{
	struct vector3 start = vector4_project(p1->coord);
	struct vector3 end = vector4_project(p2->coord);
	struct line_seg seg = clip_make_line_seg(&start, &end);
	if (seg.dir.x == 0 && seg.dir.y == 0)
	{
		// Degenerate line; clip as a point
		if (!clip_point(p1))
			return false;

		*clip_p1 = *p1;
		*clip_p2 = *p2;
		return true;
	}

	if (!clip_line_seg_unit_cube(&seg))
		return false;

	if (seg.start_t < 1)
	{
		clip_p1->coord.xyz = vector3_lerp(start, end, seg.start_t);
		clip_p1->coord.w = 1;
		clip_p1->color = vector4_lerp(p1->color, p2->color, seg.start_t);
	}
	else
		*clip_p1 = *p1;

	if (seg.end_t > 0)
	{
		clip_p2->coord.xyz = vector3_lerp(start, end, seg.end_t);
		clip_p2->coord.w = 1;
		clip_p2->color = vector4_lerp(p1->color, p2->color, seg.end_t);
	}
	else
		*clip_p2 = *p2;

	return true;
}

static bool
clip_vertex_unit_plane(const struct device_vertex *v, const struct vector3 *pnorm)
{
	struct vector3 p = vector4_project(v->coord);
	struct vector3 edge_to_p = vector3_sub(p, *pnorm);
	scalar_t num = vector3_dot(*pnorm, edge_to_p);
	return (num <= 0);
}

static struct device_vertex
clip_line_unit_plane(const struct device_vertex *v1, const struct device_vertex *v2, const struct vector3 *pnorm)
{
	struct vector3 p1 = vector4_project(v1->coord);
	struct vector3 p2 = vector4_project(v2->coord);
	struct vector3 dir = vector3_sub(p2, p1);
	scalar_t norm_dot_dir = vector3_dot(*pnorm, dir);
	assert(norm_dot_dir != 0);
	struct vector3 edge_to_p1 = vector3_sub(p1, *pnorm);
	scalar_t num = vector3_dot(*pnorm, edge_to_p1);
	scalar_t denom = -norm_dot_dir;
	scalar_t t = num / denom;
	struct device_vertex v;
	v.coord.xyz = vector3_lerp(p1, p2, t);
	v.coord.w = 1;
	v.color = vector4_lerp(v1->color, v2->color, t);
	return v;
}

static u_int
clip_polygon_unit_plane(const struct device_vertex *verts, u_int num_verts,
                        const struct vector3 *pnorm,
                        struct device_vertex *clipped_verts)
{
	u_int num_clipped_verts = 0;
	const struct device_vertex *start_v = &(verts[num_verts - 1]);
	for (u_int vert_index = 0; vert_index < num_verts; ++vert_index)
	{
		const struct device_vertex *end_v = &(verts[vert_index]);
		if (clip_vertex_unit_plane(end_v, pnorm))
		{
			if (clip_vertex_unit_plane(start_v, pnorm))
				clipped_verts[num_clipped_verts++] = *end_v;
			else
			{
				clipped_verts[num_clipped_verts++] = clip_line_unit_plane(start_v, end_v, pnorm);
				clipped_verts[num_clipped_verts++] = *end_v;
			}
		}
		else if (clip_vertex_unit_plane(start_v, pnorm))
			clipped_verts[num_clipped_verts++] = clip_line_unit_plane(start_v, end_v, pnorm);

		start_v = end_v;
	}
	return num_clipped_verts;
}

// Sutherland-Hodgeman Polygon Clip, unoptimized version
u_int
clip_polygon(const struct device_vertex *verts, u_int num_verts, struct device_vertex *clipped_verts)
{
	bcopy(verts, clipped_verts, sizeof(*verts) * num_verts);
	u_int num_clipped_verts = num_verts;

	for (u_int plane_index = 0; plane_index < sizeof(clip_planes) / sizeof(clip_planes[0]); ++plane_index)
	{
		struct device_vertex temp_verts[MAX_PRIMITIVE_VERTICES];
		num_clipped_verts = clip_polygon_unit_plane(clipped_verts, num_clipped_verts,
		                                            &(clip_planes[plane_index]),
		                                            temp_verts);
		bcopy(temp_verts, clipped_verts, sizeof(temp_verts[0]) * num_clipped_verts);
	}

	/*
	for (u_int vert_index = 0; vert_index < num_clipped_verts; ++vert_index)
		assert(clip_point(&(clipped_verts[vert_index])));
	 */

	return num_clipped_verts;
}
