#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/CGLContext.h>
#include <OpenGL/CGLCurrent.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "render.h"
#include "macro.h"
#include "draw.h"

struct lit_vertex
{
	struct vector3 world_norm;
	struct vector3 light_dirs[MAX_LIGHTS];
	struct vector3 lighting_eye_dir;
};

struct drawable *drawable;
static const struct vector4 origin = {.x = 0, .y = 0, .z = 0, .w = 1};
GLuint primitive_index;

GLIContext debug_rend;
GLIFunctionDispatch *debug_disp;
enum {DEBUG_PROJECTION, DEBUG_FRONT, DEBUG_LEFT, DEBUG_TOP, DEBUG_NMODES} debug_mode = 0;
static GLint debug_primitive_index = -1;
static GLint debug_light_index = -1;
static GLboolean debug_color = GL_FALSE;
static GLdouble debug_zoom = 0;
static struct matrix4x4 debug_proj;

void
render_init_debug(void)
{
	render_update_debug_title();
	CGLContextObj context = CGLGetCurrentContext();
	debug_rend = context->rend;
	debug_disp = &(context->disp);

	glMatrixMode(GL_PROJECTION);
	glOrtho(-1, 1, -1, 1, -10, 10);
	glMatrixMode(GL_MODELVIEW);
}

void
render_update_debug_title(void)
{
	char *mode = NULL;
	switch (debug_mode)
	{
		case DEBUG_FRONT: mode = "Front"; break;
		case DEBUG_LEFT: mode = "Left"; break;
		case DEBUG_TOP: mode = "Top"; break;
		case DEBUG_PROJECTION: mode = "Projection"; break;
		case DEBUG_NMODES: break;
	}
	char title[64];
	size_t len;
	len = snprintf(title, sizeof(title), "Debug - %s @ %.0f%%", mode, pow(2, debug_zoom) * 100);
	if (debug_light_index != -1)
		len+= snprintf(title + len, sizeof(title) - len, ", Light %u", debug_light_index);
	if (debug_primitive_index != -1)
		len+= snprintf(title + len, sizeof(title) - len, ", Primitive %u", debug_primitive_index);
	if (debug_color)
		len+= snprintf(title + len, sizeof(title) - len, ", Color");
	glutSetWindowTitle(title);
}

void
render_debug_key(u_char key, int x, int y)
{
	switch (key)
	{
		case '\t':
			debug_mode = (debug_mode + 1) % DEBUG_NMODES;
			break;
		case 0x19:
			debug_mode = (debug_mode + DEBUG_NMODES - 1) % DEBUG_NMODES;
			break;
		case 'c':
			debug_color = !debug_color;
			break;
		case 'p':
			++debug_primitive_index;
			break;
		case 'P':
			--debug_primitive_index;
			break;
		case 'l':
			if (++debug_light_index == MAX_LIGHTS)
				debug_light_index = -1;
			break;
		case 'L':
			if (debug_light_index == -1)
				debug_light_index = MAX_LIGHTS;
			--debug_light_index;
			break;
		case 'z':
			debug_zoom = fmin(3.0, debug_zoom + 0.1);
			break;
		case 'Z':
			debug_zoom = fmax(-6.0, debug_zoom - 0.1);
			break;
		default:
			return;
	}

	render_update_debug_title();
	glutPostRedisplay();
}

static void
render_update_debug_proj(const struct matrix4x4 proj)
{
	GLfloat scale = powf(2, debug_zoom);
	struct matrix4x4 scaling = matrix4x4_make_scaling(scale, scale, scale);
	switch (debug_mode)
	{
		case DEBUG_FRONT:
			debug_proj = matrix4x4_make_scaling(scale, scale, scale);
			break;
		case DEBUG_LEFT:
			debug_proj = matrix4x4_make_rotation(90, 0, 1, 0);
			debug_proj = matrix4x4_mult_matrix4x4(debug_proj, scaling);
			break;
		case DEBUG_TOP:
			debug_proj = matrix4x4_make_rotation(90, 1, 0, 0);
			debug_proj = matrix4x4_mult_matrix4x4(debug_proj, scaling);
			break;
		case DEBUG_PROJECTION:
			debug_proj = matrix4x4_mult_matrix4x4(scaling, proj);
			break;
		case DEBUG_NMODES:
			break;
	}
}

static void
render_debug_verts(struct vector4 *world_verts, u_int num_vertices)
{
	debug_disp->begin(debug_rend, GL_LINE_LOOP);
	debug_disp->color3f(debug_rend, 1, 1, 1);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		struct vector4 debug_pos = matrix4x4_mult_vector4(debug_proj, world_verts[i]);
		debug_disp->vertex4dv(debug_rend, debug_pos.v);
	}
	debug_disp->end(debug_rend);
}

static void
render_debug_line(const struct vector4 *v1, const struct vector4 *v2, GLfloat r, GLfloat g, GLfloat b)
{
	debug_disp->color3f(debug_rend, r, g, b);
	debug_disp->begin(debug_rend, GL_LINES);
	struct vector4 debug_v = matrix4x4_mult_vector4(debug_proj, *v1);
	debug_disp->vertex4dv(debug_rend, debug_v.v);
	debug_v = matrix4x4_mult_vector4(debug_proj, *v2);
	debug_disp->vertex4dv(debug_rend, debug_v.v);
	debug_disp->end(debug_rend);
}

static void
render_axes_debug()
{
	debug_disp->begin(debug_rend, GL_LINES);

	struct vector4 view_origin = matrix4x4_mult_vector4(debug_proj, origin);

	const GLdouble axes_size = 0.2;
	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin.v);
	struct vector4 right = {.x = axes_size, .y = 0, .z = 0, .w = 1};
	right = matrix4x4_mult_vector4(debug_proj, right);
	debug_disp->color3f(debug_rend, 0.5, 0.5, 1);
	debug_disp->vertex4dv(debug_rend, right.v);

	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin.v);
	struct vector4 up = {.x = 0, .y = axes_size, .z = 0, .w = 1};
	up = matrix4x4_mult_vector4(debug_proj, up);
	debug_disp->color3f(debug_rend, 1.0, 0.5, 0.5);
	debug_disp->vertex4dv(debug_rend, up.v);

	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin.v);
	struct vector4 forward = {.x = 0, .y = 0, .z = axes_size, .w = 1};
	forward = matrix4x4_mult_vector4(debug_proj, forward);
	debug_disp->color3f(debug_rend, 0.5, 1, 0.5);
	debug_disp->vertex4dv(debug_rend, forward.v);

	debug_disp->end(debug_rend);
}

static void
render_frustum_debug(struct matrix4x4 proj)
{
	debug_disp->point_size(debug_rend, 5);
	debug_disp->begin(debug_rend, GL_POINTS);
	debug_disp->color3f(debug_rend, 0, 1, 0);
	struct vector4 eye_pos = matrix4x4_mult_vector4(debug_proj, origin);
	debug_disp->vertex4dv(debug_rend, eye_pos.v);
	debug_disp->end(debug_rend);
	debug_disp->point_size(debug_rend, 1);

	static struct vector2 edges_xy[4] =
			{
					{.v = {-1, -1}},
					{.v = {-1, 1}},
					{.v = {1, 1}},
					{.v = {1, -1}}
			};
	struct matrix4x4 proj_inv = matrix4x4_invert(proj);
	struct matrix4x4 trans = matrix4x4_mult_matrix4x4(debug_proj, proj_inv);
	debug_disp->color3f(debug_rend, 0, 1, 1);
	for (u_int i = 0; i < number_of(edges_xy); ++i)
	{
		debug_disp->begin(debug_rend, GL_LINE_STRIP);

		struct vector4 next_near_corner;
		next_near_corner.xy = edges_xy[(i + 1) % number_of(edges_xy)];
		next_near_corner.z = -1;
		next_near_corner.w = 1;
		struct vector4 trans_next_near_corner = matrix4x4_mult_vector4(trans, next_near_corner);
		debug_disp->vertex4dv(debug_rend, trans_next_near_corner.v);

		struct vector4 near_corner;
		near_corner.xy = edges_xy[i];
		near_corner.z = -1;
		near_corner.w = 1;
		struct vector4 trans_near_corner = matrix4x4_mult_vector4(trans, near_corner);
		debug_disp->vertex4dv(debug_rend, trans_near_corner.v);

		struct vector4 far_corner = near_corner;
		far_corner.z = 1;
		struct vector4 trans_far_corner = matrix4x4_mult_vector4(trans, far_corner);
		debug_disp->vertex4dv(debug_rend, trans_far_corner.v);

		struct vector4 next_far_corner = next_near_corner;
		next_far_corner.z = 1;
		struct vector4 trans_next_far_corner = matrix4x4_mult_vector4(trans, next_far_corner);
		debug_disp->vertex4dv(debug_rend, trans_next_far_corner.v);

		debug_disp->end(debug_rend);
	}
}

static void
render_light_vertex(const struct vector4 *world_pos, const struct vector3 *norm,
                    const struct lighting *lighting, const struct matrix4x4 inverse_modelview,
                    struct lit_vertex *lit_vertex)
{
	struct vector4 norm4;
	norm4.xyz = *norm;
	norm4.w = 0;
	lit_vertex->world_norm = matrix4x4_mult_vector4(inverse_modelview, norm4).xyz;
	if (lighting->normalize)
		lit_vertex->world_norm = vector3_norm(lit_vertex->world_norm);
	else
		vector3_check_norm(lit_vertex->world_norm, "world_norm");

	if (debug_rend)
	{
		struct vector4 vert_norm;
		vert_norm.xyz = vector3_mult_scalar(lit_vertex->world_norm, world_pos->w / 2);
		vert_norm.w = world_pos->w;
		vert_norm.xyz = vector3_add(world_pos->xyz, vert_norm.xyz);
		render_debug_line(world_pos, &vert_norm, 0, 1, 1);
	}

	if (lighting->local_viewer)
	{
		struct vector3 eye_dir = vector3_sub(origin.xyz, world_pos->xyz);
		lit_vertex->lighting_eye_dir = vector3_norm(eye_dir);
	}
	else
		lit_vertex->lighting_eye_dir = (struct vector3){.x = 0, .y = 0, .z = 1};

	for (GLuint i = 0; i < number_of(lighting->lights); ++i)
	{
		if (!lighting->lights[i].enabled)
			continue;

		if (lighting->lights[i].pos.w == 0)
			lit_vertex->light_dirs[i] = vector3_norm(lighting->lights[i].pos.xyz);
		else
		{
			struct vector3 light_dir = vector3_sub(lighting->lights[i].pos.xyz, world_pos->xyz);
			lit_vertex->light_dirs[i] = vector3_norm(light_dir);
		}

		if (debug_rend && debug_light_index == (GLint)i)
		{
			render_debug_line(world_pos, &(lighting->lights[i].pos), 1, 0.5, 0.5);
			struct vector4 vert_light;
			vert_light.xyz = vector3_mult_scalar(lit_vertex->light_dirs[i], world_pos->w / 2);
			vert_light.w = world_pos->w;
			vert_light.xyz = vector3_add(world_pos->xyz, vert_light.xyz);
			render_debug_line(world_pos, &vert_light, 1, 1, 0);

			struct vector4 vert_eye;
			vert_eye.xyz = vector3_mult_scalar(lit_vertex->lighting_eye_dir, world_pos->w / 2);
			vert_eye.w = world_pos->w;
			vert_eye.xyz = vector3_add(world_pos->xyz, vert_eye.xyz);
			render_debug_line(world_pos, &vert_eye, 0, 1, 0);
		}
	}
}

static struct vector4
render_light_pixel(const struct lighting *lighting,
                   const struct material material,
                   const struct lit_vertex *lit_vertex)
{
	struct vector4 color;

	// Emission
	color.rgb = material.emission.rgb;
	color.a = material.diffuse.a;

	// Global ambient
	struct vector3 ambient = vector3_mult_vector3(lighting->global_ambient.rgb, material.ambient.rgb);
	color.rgb = vector3_add(color.rgb, ambient);

	for (GLuint light_index = 0; light_index < number_of(lighting->lights); ++light_index)
	{
		if (!lighting->lights[light_index].enabled)
			continue;

		// Ambient
		ambient = vector3_mult_vector3(lighting->lights[light_index].ambient.rgb, material.ambient.rgb);
		color.rgb = vector3_add(color.rgb, ambient);

		// Diffuse
		GLdouble cos_theta = vector3_dot(lit_vertex->world_norm, lit_vertex->light_dirs[light_index]);
		GLdouble diff_mix = fmax(0, cos_theta);
		struct vector3 diffuse;
		diffuse = vector3_mult_scalar(lighting->lights[light_index].diffuse.rgb, diff_mix);
		//vec3_print(diffuse);
		diffuse = vector3_mult_vector3(diffuse, material.diffuse.rgb);
		//vec3_print(color);
		color.rgb = vector3_add(color.rgb, diffuse);

		// Specular
		struct vector3 half_dir = vector3_add(lit_vertex->light_dirs[light_index], lit_vertex->lighting_eye_dir);
		half_dir = vector3_norm(half_dir);
		GLdouble cos_theta_half = vector3_dot(lit_vertex->world_norm, half_dir);
		//fprintf(stderr, "cos_theta_half = %g\n", cos_theta_half);
		GLdouble spec_mix = pow(fmax(0, cos_theta_half), material.shininess);
		//fprintf(stderr, "spec_mix = %g\n", spec_mix);
		struct vector3 specular = vector3_mult_scalar(lighting->lights[light_index].specular.rgb, spec_mix);
		specular = vector3_mult_vector3(specular, material.specular.rgb);
		//vec3_print(specular);
		color.rgb = vector3_add(color.rgb, specular);
	}
	color.rgb = vector3_clamp(color.rgb);
	return color;
}

static struct vector4
render_shade_vertex(const struct render_options *options, const struct vector4 *world_pos, const struct vertex *vertex)
{
	if (options->lighting_enabled)
	{
		struct lit_vertex lit_vertex;
		render_light_vertex(world_pos, &(vertex->norm),
		                    &(options->lighting), options->modelview.inverse_trans,
		                    &lit_vertex);
		return render_light_pixel(&(options->lighting), vertex->mat, &lit_vertex);
	}
	else
		return vertex->mat.color;
}

void
render_primitive(const struct render_options *options,
                 struct vertex *vertices,
                 u_int *indices,
                 u_int num_vertices)
{
	if (debug_primitive_index == -1 || primitive_index == (GLuint)debug_primitive_index)
	{
		if (debug_rend)
			render_update_debug_proj(options->proj);

		struct vector4 world_verts[MAX_PRIMITIVE_VERTICES];
		struct device_vertex device_vertices[MAX_PRIMITIVE_VERTICES];
		for (u_int i = 0; i < num_vertices; ++i)
		{
			struct vertex vertex = vertices[indices[i]];
			world_verts[i] = matrix4x4_mult_vector4(options->modelview.matrix, vertex.pos);
			device_vertices[i].coord = matrix4x4_mult_vector4(options->proj, world_verts[i]);
		}

		if (debug_rend)
			render_debug_verts(world_verts, num_vertices);

		// TODO: Trivial accept/reject clipping

		if (options->smooth_shading)
		{
			for (u_int i = 0; i < num_vertices; ++i)
			{
				struct vertex *vertex = &(vertices[indices[i]]);
				device_vertices[i].color = render_shade_vertex(options, &(world_verts[i]), vertex);
			}
		}
		else
		{
			u_int flat_vertex_index;
			if (num_vertices > 3)
				flat_vertex_index = indices[0];
			else
				flat_vertex_index = indices[num_vertices - 1];

			struct vertex *flat_vertex = &(vertices[flat_vertex_index]);
			struct vector4 flat_color = render_shade_vertex(options, &(world_verts[flat_vertex_index]), flat_vertex);

			for (u_int i = 0; i < num_vertices; ++i)
				device_vertices[i].color = flat_color;
		}

		draw_primitive(drawable, options->draw_options, device_vertices, num_vertices);

		if (debug_rend)
		{
			render_axes_debug();
			if (debug_mode != DEBUG_PROJECTION)
				render_frustum_debug(options->proj);
		}
	}

	++primitive_index;
}

