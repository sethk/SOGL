//
// Created by Seth Kingsley on 6/25/18.
//

#include "window_null.h"
#include "raster.h"
#include "matrix.h"
#include <gtest/gtest.h>

static struct drawable *drawable;

static void
test_scan_point(scalar_t x, scalar_t y, int expect_x, int expect_y, u_int view_x = 5)
{
	draw_set_view(drawable, view_x, 5, 14, 6);
	struct window_vertex vertex = {.coord = {.x = x, .y = y, .z = 0.5}};
	drawable->num_spans = 0;
	raster_scan_point(drawable, &vertex);
	if (expect_x != -1)
	{
		ASSERT_EQ(drawable->num_spans, 1);
		struct raster_span *span = &(drawable->spans[0]);
		ASSERT_EQ(span->left_x, expect_x);
		ASSERT_EQ(span->y, expect_y);
		ASSERT_EQ(span->x_delta, 1);
		ASSERT_EQ(span->left_depth, 0.5);
		ASSERT_EQ(span->depth_delta, 0);
	}
	else
		ASSERT_EQ(drawable->num_spans, 0);
}

TEST(ScanTest, ScanPoint1_view2)
{
	struct window *window = window_create_null();
	drawable = draw_create(window);
	draw_reshape(drawable, 5 + 16 + 5, 5 + 8 + 5);
	test_scan_point(6, 4.5, 5, 4, 2);
}

TEST(ScanTest, ScanPoints_All)
{
	struct window *window = window_create_null();
	drawable = draw_create(window);
	draw_reshape(drawable, 5 + 16 + 5, 5 + 8 + 5);
	for (u_int view_x = 0; view_x < 6; ++view_x)
	{
		test_scan_point(0, 6, -1, -1, view_x);
		test_scan_point(1.5, 4.5, 1, 4, view_x);
		test_scan_point(3.5, 4, 3, 4, view_x);
		test_scan_point(6, 4.5, 5, 4, view_x);
		test_scan_point(9, 4, 8, 4, view_x);
		test_scan_point(12, 4, 11, 4, view_x);
		test_scan_point(13, 4, 12, 4, view_x);
		test_scan_point(14, 6, -1, -1, view_x);
		test_scan_point(0.25, 2.75, 0, 2, view_x);
		test_scan_point(3.5, 1.25, 3, 1, view_x);
		test_scan_point(5.25, 1.5, 5, 1, view_x);
		test_scan_point(8.75, 1.25, 8, 1, view_x);
		test_scan_point(0, 0, -1, -1, view_x);
		test_scan_point(11, 0, 10, 0, view_x);
		test_scan_point(14, 0, 13, 0, view_x);
	}
}
