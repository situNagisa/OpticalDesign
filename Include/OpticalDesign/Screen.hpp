#pragma once

#include "NGS/NGS.h"
#include "opencv2/opencv.hpp"
#include "OpticalDesign/DeviceConfig.h"

namespace devices {


	inline bool Screen::Open() { return true; }
	inline bool Screen::IsOpened()const { return true; }
	inline void Screen::Close() {}

	inline void Screen::Update() {}

	inline void Screen::Showconst cv::Mat& image) {
		cv::imshow("screen", image);
	}
}