//
// Created by Seth Kingsley on 8/29/18.
//

#include "../src/raster.h"
#include "../src/clip.h"
#include <cmath>
#include <gtest/gtest.h>

#pragma GCC diagnostic ignored "-Wunused-const-variable"

using namespace std;

ostream &operator<<(ostream &os, const struct vector4 &v)
{
	return os << '{' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << '}';
}

ostream &operator<<(ostream &os, const struct vector3 &v)
{
	return os << '{' << v.x << ", " << v.y << ", " << v.z << '}';
}

ostream &operator<<(ostream &os, const struct device_vertex &v)
{
	return os << "{.coord = " << v.coord << ", .color = " << v.color << '}';
}

bool operator==(const struct vector4 &v1, const struct vector4 &v2)
{
	// TODO: Differing w term
	return (fabs(v1.x - v2.x) < 1e-7 &&
			fabs(v1.y - v2.y) < 1e-7 &&
			fabs(v1.z - v2.z) < 1e-7 &&
			fabs(v1.w - v2.w) < 1e-7);
}

bool operator==(const struct vector3 &v1, const struct vector3 &v2)
{
	return (fabs(v1.x - v2.x) < 1e-7 &&
			fabs(v1.y - v2.y) < 1e-7 &&
			fabs(v1.z - v2.z) < 1e-7);
}

bool operator==(const struct device_vertex &v1, const struct device_vertex &v2)
{
	return (v1.coord == v2.coord && v1.color == v2.color);
}

class ClipTest : public ::testing::Test
{
protected:
	void
	TestClipLine(const struct vector3 &c1,
	             const struct vector3 &c2,
	             bool clip_visible,
	             const struct vector3 &clip_c1,
	             const struct vector3 &clip_c2)
	{
		struct device_vertex v1 = {.coord.w = 1, .color.v = {0, 0, 0, 0}};
		v1.coord.xyz = c1;
		struct device_vertex v2 = {.coord.w = 1, .color.v = {0, 0, 0, 0}};
		v2.coord.xyz = c2;
		bool visible = clip_line(&v1, &v2, &v1, &v2);
		ASSERT_EQ(visible, clip_visible);
		if (visible && clip_visible)
		{
			ASSERT_EQ(v1.coord.xyz, clip_c1);
			ASSERT_EQ(v2.coord.xyz, clip_c2);
		}
	}

	void
	TestClipTriangle(const struct vector3 &c1, const struct vector3 &c2, const struct vector3 &c3,
			initializer_list<const struct vector3> expect_clip_coords)
	{
		struct device_vertex verts[3] =
				{
						{.coord.w = 1, .color.v = {0, 0, 0, 0}},
						{.coord.w = 1, .color.v = {0, 0, 0, 0}},
						{.coord.w = 1, .color.v = {0, 0, 0, 0}}
				};
		verts[0].coord.xyz = c1;
		verts[1].coord.xyz = c2;
		verts[2].coord.xyz = c3;
		struct device_vertex clipped_verts[6];
		u_int num_clip_verts = clip_polygon(verts, 3, clipped_verts);
		u_int expect_index = 0;
		for (const struct vector3 &expect_clip : expect_clip_coords)
		{
			ASSERT_GT(num_clip_verts, expect_index);
			struct vector4 expect_clip_coord4;
			expect_clip_coord4.xyz = expect_clip;
			expect_clip_coord4.w = 1;
			ASSERT_EQ(clipped_verts[expect_index].coord, expect_clip_coord4);
			++expect_index;
		}
		ASSERT_EQ(num_clip_verts, expect_clip_coords.size());
	}
};

static const struct vector3 Top1 = {.v = {0.5, 1.5, 0}},
		TopFront1 = {.v = {-0.5, 2.5, 2.5}},
		Left1 = {.v = {-1.5, 0.5, 0}},
		Left2 = {.v = {-2.5, -0.5, 0}},
		Bottom1 = {.v = {-0.5, -1.5, 0}},
		BottomBack1 = {.v = {0.5, -2.5, -2.5}},
		Right1 = {.v = {1.5, -0.5, 0}},
		Right2 = {.v = {2.5, 0.5, 0}},
		Inside1 = {.v = {-0.5, 0.5, 0}},
		Inside2 = {.v = {0.5, -0.5, 0}},
		TopEdge1 = {.v = {0, 1, 0}},
		TopEdge2 = {.v = {0.5, 1, 0}},
		TopFrontEdge1 = {.v = {-0.2, 1, 1}},
		LeftEdge1 = {.v = {-1, 0.25, 0}},
		LeftEdge2 = {.v = {-1, 1 / 3.0f, 0}},
		BottomEdge1 = {.v = {0, -1, 0}},
		BottomBackEdge1 = {.v = {0.2, -1, -1}},
		RightEdge1 = {.v = {1, -0.25, 0}},
		RightEdge2 = {.v = {1, -1 / 3.0, 0}},
		RightEdge3 = {.v = {1, 0, 0}},
		TopRight1 = {.v = {2.5, 1.5, 0}};

TEST_F(ClipTest, TestLines_Inside_Inside)
{
	TestClipLine(Inside1, Inside2, true, Inside1, Inside2);
	TestClipLine(Inside2, Inside1, true, Inside2, Inside1);
}

TEST_F(ClipTest, TestLines_Left_Inside)
{
	TestClipLine(Left1, Inside2, true, LeftEdge1, Inside2);
	TestClipLine(Inside2, Left1, true, Inside2, LeftEdge1);
}

TEST_F(ClipTest, TestLines_Left1_Left2)
{
	TestClipLine(Left1, Left2, false, Left1, Left2);
	TestClipLine(Left2, Left1, false, Left2, Left1);
}

TEST_F(ClipTest, TestLines)
{
	TestClipLine(Inside1, Right1, true, Inside1, RightEdge1);
	TestClipLine(Right1, Inside1, true, RightEdge1, Inside1);

	TestClipLine(Right1, Right2, false, Right1, Right2);
	TestClipLine(Right2, Right1, false, Right2, Right1);
}

TEST_F(ClipTest, TestLines_Right_Left)
{
	TestClipLine(Right1, Left1, true, RightEdge2, LeftEdge2);
	TestClipLine(Left1, Right1, true, LeftEdge2, RightEdge2);
}

TEST_F(ClipTest, TestLines_Top_Inside)
{
	TestClipLine(Top1, Inside1, true, TopEdge1, Inside1);
	TestClipLine(Inside1, Top1, true, Inside1, TopEdge1);
}

TEST_F(ClipTest, TestLines_Bottom_Inside)
{
	TestClipLine(Bottom1, Inside2, true, BottomEdge1, Inside2);
	TestClipLine(Inside2, Bottom1, true, Inside2, BottomEdge1);
}

TEST_F(ClipTest, TestLines_Bottom_TopRight)
{
	TestClipLine(Bottom1, TopRight1, true, BottomEdge1, RightEdge3);
	TestClipLine(TopRight1, Bottom1, true, RightEdge3, BottomEdge1);
}

TEST_F(ClipTest, TestLines_TopFront_BottomBack)
{
	TestClipLine(TopFront1, BottomBack1, true, TopFrontEdge1, BottomBackEdge1);
	TestClipLine(BottomBack1, TopFront1, true, BottomBackEdge1, TopFrontEdge1);
}

TEST_F(ClipTest, TestTriangle_Upper)
{
	TestClipTriangle(Inside1, Top1, Inside2, {Inside1, TopEdge1, TopEdge2, Inside2});
}
