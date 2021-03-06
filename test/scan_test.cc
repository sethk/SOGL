//
// Created by Seth Kingsley on 6/25/18.
//

#include "../src/window_null.h"
#include "../src/raster.h"
#include "../src/matrix.h"
#include <gtest/gtest.h>

using namespace std;

ostream &operator<<(ostream &os, const struct raster_span &span)
{
	return os << "{left_x = " << span.left_x << ", y = " << span.y << ", x_delta = " << span.x_delta
			<< ", left_depth = " << span.left_depth << '}';
}

typedef vector<struct raster_span> raster_spans;

class ScanTest : public ::testing::Test
{
protected:
	struct drawable *drawable;

	ScanTest()
	{
		struct window *window = window_create_null();
		drawable = draw_create(window);
		draw_reshape(drawable, 5 + 16 + 5, 5 + 16 + 5);
	}

	void AssertSpans(initializer_list<struct raster_span> expect_spans_init)
	{
		struct raster_span expect_spans[expect_spans_init.size()];
		u_int expect_num_spans = 0;
		for (const struct raster_span &span : expect_spans_init)
			expect_spans[expect_num_spans++] = span;
		AssertSpans(expect_spans, expect_num_spans);
	}

	void AssertSpans(struct raster_span expect_spans[], u_int expect_num_spans)
	{
		for (u_int i = 0; i < expect_num_spans; ++i)
		{
			ASSERT_GT(drawable->num_spans, i);
			struct raster_span *span = &(drawable->spans[i]);
			struct raster_span *expect_span = &(expect_spans[i]);
			ASSERT_TRUE(span->y == expect_span->y &&
			            span->left_x == expect_span->left_x &&
			            span->x_delta == expect_span->x_delta)
										<< "span[" << i << "]" << *span << " != expect_span" << *expect_span;
		}
		ASSERT_EQ(drawable->num_spans, expect_num_spans);
	}

	void TestScanPoint(scalar_t x, scalar_t y, int expect_x, int expect_y, u_int view_x = 5)
	{
		draw_set_view(drawable, view_x, 5, 14, 6);
		struct window_vertex vertex = {.coord = {.x = x, .y = y, .z = 0.5}};
		drawable->num_spans = 0;
		raster_scan_point(drawable, &vertex);
		ASSERT_EQ(drawable->num_spans, 1);
		struct raster_span *span = &(drawable->spans[0]);
		ASSERT_EQ(span->left_x, expect_x);
		ASSERT_EQ(span->y, expect_y);
		ASSERT_EQ(span->x_delta, 1);
		ASSERT_EQ(span->left_depth, 0.5);
		ASSERT_EQ(span->depth_delta, 0);
	}

	void TestScanLine(scalar_t x1, scalar_t y1,
	                  scalar_t x2, scalar_t y2,
	                  initializer_list<struct raster_span> expect_spans_init)
	{
		draw_set_view(drawable, 5, 5, 16, 16);
		drawable->num_spans = 0;
		struct window_vertex v1 = {.coord = {.x = x1, .y = y1, .z = 0.5}};
		struct window_vertex v2 = {.coord = {.x = x2, .y = y2, .z = 0.5}};
		raster_scan_line(drawable, &v1, &v2);
		AssertSpans(expect_spans_init);
	}

	void TestScanTriangle(scalar_t x1, scalar_t y1,
	                      scalar_t x2, scalar_t y2,
	                      scalar_t x3, scalar_t y3,
	                      u_int expect_num_spans, struct raster_span expect_spans[])
	{
		draw_set_view(drawable, 5, 5, 16, 8);
		struct window_vertex vertices[3] =
		{
				{.coord = {.x = x1, .y = y1, .z = 0.5}},
				{.coord = {.x = x2, .y = y2, .z = 0.5}},
				{.coord = {.x = x3, .y = y3, .z = 0.5}}
		};
		drawable->num_spans = 0;
		raster_scan_triangle(drawable, vertices);
		AssertSpans(expect_spans, expect_num_spans);
	}
};

TEST_F(ScanTest, ScanPoint1_view2)
{
#if DIRECTX_POINT_RASTER
	TestScanPoint(6, 4.5, 5, 4, 2);
#else
	TestScanPoint(6, 4.5, 6, 4, 2);
#endif // DIRECTX_POINT_RASTER
}

TEST_F(ScanTest, ScanPoints_All)
{
	for (u_int view_x = 0; view_x < 6; ++view_x)
	{
#if DIRECTX_POINT_RASTER
		TestScanPoint(0, 6, -1, 6, view_x);
		TestScanPoint(1.5, 4.5, 1, 4, view_x);
		TestScanPoint(3.5, 4, 3, 4, view_x);
		TestScanPoint(6, 4.5, 5, 4, view_x);
		TestScanPoint(9, 4, 8, 4, view_x);
		TestScanPoint(12, 4, 11, 4, view_x);
		TestScanPoint(13, 4, 12, 4, view_x);
		TestScanPoint(14, 6, 13, 6, view_x);
		TestScanPoint(0.25, 2.75, 0, 2, view_x);
		TestScanPoint(3.5, 1.25, 3, 1, view_x);
		TestScanPoint(5.25, 1.5, 5, 1, view_x);
		TestScanPoint(8.75, 1.25, 8, 1, view_x);
		TestScanPoint(0, 0, -1, 0, view_x);
		TestScanPoint(11, 0, 10, 0, view_x);
		TestScanPoint(14, 0, 13, 0, view_x);
#else
		TestScanPoint(0, 6, 0, 6, view_x);
		TestScanPoint(1.5, 4.5, 1, 4, view_x);
		TestScanPoint(3.5, 4, 3, 4, view_x);
		TestScanPoint(6, 4.5, 6, 4, view_x);
		TestScanPoint(9, 4, 9, 4, view_x);
		TestScanPoint(12, 4, 12, 4, view_x);
		TestScanPoint(13, 4, 13, 4, view_x);
		TestScanPoint(14, 6, 14, 6, view_x);
		TestScanPoint(0.25, 2.75, 0, 2, view_x);
		TestScanPoint(3.5, 1.25, 3, 1, view_x);
		TestScanPoint(5.25, 1.5, 5, 1, view_x);
		TestScanPoint(8.75, 1.25, 8, 1, view_x);
		TestScanPoint(0, 0, 0, 0, view_x);
		TestScanPoint(11, 0, 11, 0, view_x);
		TestScanPoint(14, 0, 14, 0, view_x);
#endif // DIRECTX_POINT_RASTER
	}
}

TEST_F(ScanTest, ScanTriangle1)
{
	struct raster_span spans1[] =
			{
					{.left_x = 2, .y = 4, .x_delta = 1},
					{.left_x = 1, .y = 5, .x_delta = 4},
					{.left_x = 1, .y = 6, .x_delta = 2}
			};
	TestScanTriangle(1, 7, 2, 4, 6, 6, sizeof(spans1) / sizeof(spans1[0]), spans1);
}

TEST_F(ScanTest, ScanTriangle2)
{
	struct raster_span spans2[] =
			{
					{.left_x = 9, .y = 6, .x_delta = 2}
			};
	TestScanTriangle(9.75, 7.25, 7.75, 5.5, 11.75, 5.5, sizeof(spans2) / sizeof(spans2[0]), spans2);
}

TEST_F(ScanTest, ScanTriangle_All)
{
	//TestScanTriangle(4.5, 7.5, 4.5, 7.5, 4.5, 7.5, 0, NULL);
	TestScanTriangle(6.25, 7.75, 5.25, 6.75, 6.25, 6.75, 0, NULL);
	TestScanTriangle(7.5, 7.5, 6.5, 6.5, 7.5, 6.5, 0, NULL);

	struct raster_span spans3[] =
			{
					{.left_x = 13, .y = 6, .x_delta = 2},
					{.left_x = 14, .y = 7, .x_delta = 1}
			};
	TestScanTriangle(15, 8, 13.5, 6.5, 14.5, 5.5, sizeof(spans3) / sizeof(spans3[0]), spans3);

	struct raster_span spans4[] =
			{
					{.left_x = 9, .y = 3, .x_delta = 1},
					{.left_x = 8, .y = 4, .x_delta = 3},
					{.left_x = 8, .y = 5, .x_delta = 4}
			};
	TestScanTriangle(7.75, 5.5, 9.5, 2.75, 11.75, 5.5, sizeof(spans4) / sizeof(spans4[0]), spans4);

	TestScanTriangle(13.5, 6.5, 14.5, 3.5, 14.5, 5.5, 0, NULL);

	struct raster_span spans5[] =
			{
					{.left_x = 2, .y = 2, .x_delta = 3},
					{.left_x = 5, .y = 3, .x_delta = 1}
			};
	TestScanTriangle(7, 4, 1, 2, 5, 2, sizeof(spans5) / sizeof(spans5[0]), spans5);

	struct raster_span spans6[] =
			{
					{.left_x = 6, .y = 1, .x_delta = 2},
					{.left_x = 5, .y = 2, .x_delta = 2},
					{.left_x = 6, .y = 3, .x_delta = 1}
			};
	TestScanTriangle(7, 4, 5, 2, 8, 1, sizeof(spans6) / sizeof(spans6[0]), spans6);

	struct raster_span spans7[] =
			{
					{.left_x = 7, .y = 2, .x_delta = 2},
					{.left_x = 7, .y = 3, .x_delta = 1}
			};
	TestScanTriangle(7, 4, 8, 1, 9.5, 2.5, sizeof(spans7) / sizeof(spans7[0]), spans7);

	struct raster_span spans8[] =
			{
					{.left_x = 11, .y = 2, .x_delta = 1}
			};
	TestScanTriangle(11.5, 3.5, 11.5, 1.5, 12.5, 2.5, sizeof(spans8) / sizeof(spans8[0]), spans8);

	struct raster_span spans9[] =
			{
					{.left_x = 13, .y = 1, .x_delta = 1},
					{.left_x = 13, .y = 2, .x_delta = 2}
			};
	TestScanTriangle(13.5, 2.5, 13.5, 0.5, 15.5, 2.5, sizeof(spans9) / sizeof(spans9[0]), spans9);

	struct raster_span spans10[] =
			{
					{.left_x = 14, .y = 1, .x_delta = 1}
			};
	TestScanTriangle(15.5, 2.5, 13.5, 0.5, 15.5, 0.5, sizeof(spans10) / sizeof(spans10[0]), spans10);

	struct raster_span spans11[] =
			{
					{.left_x = 9, .y = -1, .x_delta = 1},
					{.left_x = 9, .y = 0, .x_delta = 1}
			};
	TestScanTriangle(9.5, 0.5, 9.5, -1.5, 10.5, 0.5, sizeof(spans11) / sizeof(spans11[0]), spans11);
}

TEST_F(ScanTest, ScanLine1_Octant7)
{
	TestScanLine(0.5, 15.5, 1.5, 15.25, {{.left_x = 0, .y = 15, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine2_Octant6)
{
	TestScanLine(3.75, 16.1, 3.9, 14.9, {{.left_x = 3, .y = 15, .x_delta = 1}});
}

TEST_F(ScanTest, Scanline3_Octant6)
{
	TestScanLine(6, 15.5, 6.1, 15.1, {{.left_x = 5, .y = 15, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine4_Octant0)
{
	TestScanLine(7.5, 15, 7.9, 15.1, {{.left_x = 7, .y = 15, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine5_Down)
{
	TestScanLine(9.25, 15.25, 9.25, 14.75, {{.left_x = 9, .y = 15, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine6_Down)
{
	TestScanLine(11.75, 15.25, 11.75, 14.75, {{.left_x = 11, .y = 15, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine7_Octant0)
{
	TestScanLine(13.1, 15.6, 13.4, 15.9, {});
}

TEST_F(ScanTest, ScanLine8_Right)
{
	TestScanLine(14.25, 15.9, 14.75, 15.9, {{.left_x = 14, .y = 15, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine9_Octant7_Null)
{
	TestScanLine(15.6, 15.9, 15.9, 15.6, {});
}

TEST_F(ScanTest, Scanline10_Null)
{
	TestScanLine(12.2, 14.6, 12.2, 14.6, {});
}

TEST_F(ScanTest, ScanLine11_Octant3)
{
	TestScanLine(13.4, 14.1, 13.1, 14.4, {});
}

TEST_F(ScanTest, ScanLine12_Octant4)
{
	TestScanLine(15.9, 14.4, 15.6, 14.1, {});
}

TEST_F(ScanTest, ScanLine13_Octant5)
{
	TestScanLine(10.5, 14, 9, 9.5,
	             {
			             {.left_x = 10, .y = 14, .x_delta = 1},
			             {.left_x = 10, .y = 13, .x_delta = 1},
			             {.left_x = 9, .y = 12, .x_delta = 1},
			             {.left_x = 9, .y = 11, .x_delta = 1},
			             {.left_x = 9, .y = 10, .x_delta = 1}
	             });
}

TEST_F(ScanTest, ScanLine14_Octant3)
{
	TestScanLine(1.5, 13.25, 0.5, 13.5, {{.left_x = 1, .y = 13, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine15_Octant7)
{
	TestScanLine(4, 13.5, 8.5, 12, {{.left_x = 4, .y = 13, .x_delta = 2}, {.left_x = 6, .y = 12, .x_delta = 2}});
}

TEST_F(ScanTest, ScanLine16_Left)
{
	TestScanLine(15.3, 13.1, 14.9, 13.1, {});
}

TEST_F(ScanTest, ScanLine17_Octant7)
{
	TestScanLine(2.5, 12, 4.5, 10, {{.left_x = 2, .y = 12, .x_delta = 1}, {.left_x = 3, .y = 11, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine18_Null)
{
	TestScanLine(12.1, 12.2, 12.1, 12.1, {});
}

TEST_F(ScanTest, ScanLine19_Null)
{
	TestScanLine(12.1, 12.9, 12.2, 12.9, {});
}

TEST_F(ScanTest, ScanLine20_Null)
{
	TestScanLine(12.2, 12.6, 12.4, 12.6, {});
}

TEST_F(ScanTest, ScanLine21_Null)
{
	TestScanLine(12.6, 12.6, 12.6, 12.4, {});
}

TEST_F(ScanTest, ScanLine22_Null)
{
	TestScanLine(12.8, 12.9, 12.9, 12.9, {});
}

TEST_F(ScanTest, ScanLine23_Null)
{
	TestScanLine(12.9, 12.2, 12.9, 12.1, {});
}

TEST_F(ScanTest, ScanLine24_Octant3_Null)
{
	TestScanLine(14.25, 12.75, 13.9, 13.1, {});
}

TEST_F(ScanTest, ScanLine25_Octant7_Null)
{
	TestScanLine(14.6, 12.9, 15.1, 12.6, {});
}

TEST_F(ScanTest, ScanLine26_Up_Null)
{
	TestScanLine(15.9, 12.9, 15.9, 13.1, {});
}

TEST_F(ScanTest, ScanLine27_Down)
{
	TestScanLine(6, 11.5, 6, 10.5, {{.left_x = 5, .y = 11, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine28_Octant7_Null)
{
	TestScanLine(14, 11.5, 14.5, 11, {});
}

TEST_F(ScanTest, ScanLine29_Octant0)
{
	TestScanLine(0.5, 10, 2.5, 12,
	{
			{.left_x = 0, .y = 10, .x_delta = 1},
		{.left_x = 1, .y = 11, .x_delta = 1},
	});
}

TEST_F(ScanTest, ScanLine30_Down)
{
	TestScanLine(6, 10.5, 6, 9.5, {{.left_x = 5, .y = 10, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine31_Octant4)
{
	TestScanLine(12.5, 10, 13, 10.5, {{.left_x = 12, .y = 10, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine32_Octant5)
{
	TestScanLine(15.5, 10, 15, 9.5, {{.left_x = 15, .y = 10, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine33_Up)
{
	TestScanLine(13.5, 9, 13.5, 10, {{.left_x = 13, .y = 9, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine34_Octant6)
{
	TestScanLine(3, 8.5, 1, 6.5,
	             {
			             {.left_x = 2, .y = 8, .x_delta = 1},
			             {.left_x = 1, .y = 7, .x_delta = 1},
	             });
}

TEST_F(ScanTest, ScanLine35_Octant7)
{
	TestScanLine(6.5, 8, 8.5, 6,
	             {
			             {.left_x = 6, .y = 8, .x_delta = 1},
			             {.left_x = 7, .y = 7, .x_delta = 1},
	             });
}

TEST_F(ScanTest, ScanLine36_Left)
{
	TestScanLine(10.5, 8, 9.5, 8, {{.left_x = 10, .y = 8, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine37_Left)
{
	TestScanLine(11.5, 8, 10.5, 8, {{.left_x = 11, .y = 8, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine38_Down)
{
	TestScanLine(13.5, 7, 13.5, 6, {{.left_x = 13, .y = 7, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine39_Octant0_Null)
{
	TestScanLine(15, 7.5, 15.5, 8, {});
}

TEST_F(ScanTest, ScanLine40_Octant0)
{
	TestScanLine(1, 6.5, 3, 8.5, {{.left_x = 1, .y = 7, .x_delta = 1}, {.left_x = 2, .y = 8, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine41_Octant4)
{
	TestScanLine(8.5, 6, 6.5, 4,
	{
		{.left_x = 8, .y = 6, .x_delta = 1},
		{.left_x = 7, .y = 5, .x_delta = 1},
	});
}

TEST_F(ScanTest, ScanLine42_Octant7)
{
	TestScanLine(10, 6.5, 12, 4.5, {{.left_x = 10, .y = 6, .x_delta = 1}, {.left_x = 11, .y = 5, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine43_Octant3_Null)
{
	TestScanLine(13, 6.5, 12.5, 7, {});
}

TEST_F(ScanTest, ScanLine44_Octant7)
{
	TestScanLine(4, 5.5, 6, 3.5, {{.left_x = 4, .y = 5, .x_delta = 1}, {.left_x = 5, .y = 4, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine45_Octant6)
{
	TestScanLine(14.1, 5.2, 12.2, 2.1,
	             {
			             {.left_x = 13, .y = 4, .x_delta = 1},
			             {.left_x = 13, .y = 3, .x_delta = 1},
			             {.left_x = 12, .y = 2, .x_delta = 1},
	             });
}

TEST_F(ScanTest, ScanLine46_Octant6)
{
	TestScanLine(16.1, 5.2, 14, 1.5,
	             {
			             {.left_x = 15, .y = 4, .x_delta = 1},
			             {.left_x = 15, .y = 3, .x_delta = 1},
			             {.left_x = 14, .y = 2, .x_delta = 1},
	             });
}

TEST_F(ScanTest, ScanLine47_Octant0)
{
	TestScanLine(1, 4.5, 3, 6.5, {{.left_x = 1, .y = 5, .x_delta = 1}, {.left_x = 2, .y = 6, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine48_Octant7)
{
	TestScanLine(2.5, 4, 4.5, 2, {{.left_x = 2, .y = 4, .x_delta = 1}, {.left_x = 3, .y = 3, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine49_Left)
{
	TestScanLine(9, 4.5, 8, 4.5, {{.left_x = 8, .y = 4, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine50_Octant6)
{
	TestScanLine(12, 4.5, 10, 2.5,
	             {
			             {.left_x = 11, .y = 4, .x_delta = 1},
			             {.left_x = 10, .y = 3, .x_delta = 1},
	             });
}

TEST_F(ScanTest, ScanLine51_Right)
{
	TestScanLine(8, 3.5, 9, 3.5, {{.left_x = 8, .y = 3, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine52_Octant6)
{
	TestScanLine(2.5, 2, 0.5, 0, {{.left_x = 2, .y = 2, .x_delta = 1}, {.left_x = 1, .y = 1, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine53_Octant6)
{
	TestScanLine(7, 2.5, 5, 0.5,
	             {
			             {.left_x = 6, .y = 2, .x_delta = 1},
			             {.left_x = 5, .y = 1, .x_delta = 1},
	             });
}

TEST_F(ScanTest, ScanLine54_Octant0)
{
	TestScanLine(3.5, 1, 4, 1.5, {{.left_x = 3, .y = 1, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine55_Right)
{
	TestScanLine(8, 1.5, 10, 1.5, {{.left_x = 8, .y = 1, .x_delta = 2}});
}

TEST_F(ScanTest, ScanLine56_Octant7)
{
	TestScanLine(10, 1.5, 11, 0.5, {{.left_x = 10, .y = 1, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine57_Left)
{
	TestScanLine(12.5, 1, 12, 1, {{.left_x = 12, .y = 1, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine58_Up)
{
	TestScanLine(3.5, 0, 3.5, 1, {{.left_x = 3, .y = 0, .x_delta = 1}});
}

TEST_F(ScanTest, ScanLine59_Octant3)
{
	TestScanLine(13.5, 0, 12.5, 1, {{.left_x = 13, .y = 0, .x_delta = 1}});
}
