#pragma once

#include "NGS/NGS.h"
#include "opencv2/opencv.hpp"
#include "box2d/box2d.h"
#include "OpticalDesign/Setting.h"

OPT_BEGIN

inline static constexpr Point TRANSFORMANT{ 0.2f, -0.2f };

namespace ngs_cv {
	inline cv::Rect Convert(const Rect& rect) {
		return { (int)rect.x,(int)rect.y,(int)rect.width,(int)rect.height };
	}

	inline Rect Convert(const cv::Rect& rect) {
		return { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };
	}

	inline cv::Point Convert(const Point& point) {
		return { (int)point.x,(int)point.y };
	}
	inline Point Convert(const cv::Point& point) {
		return { (float)point.x,(float)point.y };
	}

	inline ngs::HSV Convert(const std::array<int, 3>& hsv) {
		return { (ngs::byte)hsv[0],hsv[1] / (float)0xFF,hsv[2] / (float)0xFF };
	}
}

namespace ngs_b2 {
	inline b2Vec2 Convert(const Point& pos) {
		return b2Vec2(pos.x, pos.y);
	}
	inline Point Convert(const b2Vec2& pos) {
		return Point(pos.x, pos.y);
	}

	inline b2Vec2 TransformToBox2D(const Point& pos) {
		return ngs_b2::Convert(pos * TRANSFORMANT);
	}
	inline Point TransformToNGS(const b2Vec2& pos) {
		auto result = ngs_b2::Convert(pos) / TRANSFORMANT;
		result.x = round(result.x);
		result.y = round(result.y);
		return result;
	}
}

OPT_END