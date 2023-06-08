#pragma once

#include "NGS/NGS.h"
#include "opencv2/opencv.hpp"
#include "OpticalDesign/Config.h"

OPT_BEGIN


using Point = ngs::Point2f;
using Rect = ngs::Rectangle<float>;

constexpr ngs::Vector2D<size_t> MAZE_SIZE{ 10, 10 };
constexpr ngs::Vector2D<size_t> GRID_NUM(2 * MAZE_SIZE.x + 1, 2 * MAZE_SIZE.y + 1);

constexpr Point CAR_SIZE{ 0.25f,0.25f };
constexpr Point CAR_SIZE_HALF{ CAR_SIZE.x / 2, CAR_SIZE.y / 2 };
constexpr float CAR_AREA = CAR_SIZE.x * CAR_SIZE.y;
constexpr float CAR_MASS = 2.0f;
constexpr float CAR_DENSITY = CAR_MASS / CAR_AREA;

#if NGS_PLATFORM == NGS_WINDOWS
constexpr auto TEST_PICTURE = R"(C:\Users\86198\Pictures\test8.jpg)";
#elif NGS_PLATFORM == NGS_LINUX
constexpr auto TEST_PICTURE = "/home/nagisa/pic/test8.jpg";
#endif

constexpr float FOOT_ROTATE_ANGLE_THRESHOLD = 0.1f;
constexpr float FOOT_MOVE_DISTANCE_THRESHOLD = 0.05f;

inline constexpr bool DEBUG_EYES = false;


constexpr ngs::Vector2D<size_t> IMAGE_SIZE{ 350, 350 };

struct Treasure {
	ngs::HSV card;
	ngs::HSV pattern;
	enum class Form {
		unknown,
		circle,
		triangle,
	} form;
};
Treasure NULL_TREASURE = { {},{},Treasure::Form::unknown };

struct ColorRange {
	std::array<int, 3>
		low, high;

	cv::Mat GetMask(const cv::Mat& hsv)const {
		cv::Mat mask;
		if (high[0] > 180) {
			cv::Mat mask1, mask2;
			cv::inRange(hsv, low, std::array{ 180, high[1], high[2] }, mask1);
			cv::inRange(hsv, std::array{ 0, low[1], low[2] }, std::array{ high[0] - 180, high[1], high[2] }, mask2);
			cv::bitwise_or(mask1, mask2, mask);
			return mask;
		}
		cv::inRange(hsv, low, high, mask);
		return mask;
	}

	std::array<int, 3> GetColor()const {
		std::array<int, 3> result = {};
		for (size_t i = 0; i < result.max_size(); i++)
		{
			result[i] = (low[i] + high[i]) / 2;
		}
		return result;
	}
};

namespace color_defined {
	constexpr ColorRange
		red = { {145,0x25,0x50},{210,0xFF,0xFF} },
		green = { {37,0x20,0x20},{80,0xFF,0xFF} },
		blue = { {85,0x40,0x50},{125,0xFF,0xFF} },
		yellow = { {8,0x20,0x50},{35,0xFF,0xFF} }
	;
}

enum class Team {
	RED,
	BLUE
};

std::pair<ColorRange, ColorRange> GetTeamColorRange(Team team) {
	switch (team)
	{
	case optical_design::Team::RED:
		return { color_defined::red,color_defined::green };
		break;
	case optical_design::Team::BLUE:
		return { color_defined::blue,color_defined::yellow };
		break;
	default:
		break;
	}
}
Team GetTeamTarget(Team team) {
	switch (team)
	{
	case optical_design::Team::RED:
		return Team::BLUE;
		break;
	case optical_design::Team::BLUE:
		return Team::RED;
		break;
	default:
		break;
	}
}

using GrayData = std::array<bool, 8>;

OPT_END

NGS_BEGIN

inline bool In(const optical_design::Point& point, const optical_design::Point& A, const optical_design::Point& B) {
	if (A.x == B.x) {
		if (ngs::In(point.x, A.x - 1, A.x + 1) && ngs::In(point.y, ngs::Min(A.y, B.y), ngs::Max(A.y, B.y)))
			return true;
	}
	else if (A.y == B.y) {
		if (ngs::In(point.y, A.y - 1, A.y + 1) && ngs::In(point.x, ngs::Min(A.x, B.x), ngs::Max(A.x, B.x)))
			return true;
	}
	else
		ngs::nos.Error("invalid point");
	return false;
}

NGS_END