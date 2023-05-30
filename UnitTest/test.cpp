#include "pch.h"
#include "NGS/NGS.h"

USE_NGS;

TEST(ngs, Color) {
	constexpr ARGB32 argb(0xFF, 0xFF, 0xFF, 0xFF);
	ASSERT_EQ(argb.Alpha(), 0xFF);
	ASSERT_EQ(argb.Red(), 0xFF);
	ASSERT_EQ(argb.Green(), 0xFF);
	ASSERT_EQ(argb.Blue(), 0x00);
}
