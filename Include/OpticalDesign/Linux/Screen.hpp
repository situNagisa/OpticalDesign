#pragma once

#include "NGS/NGS.h"
#include "opencv2/opencv.hpp"
#include "OpticalDesign/DeviceConfig.h"
#include "OpticalDesign/Linux/NSLConfig.h"

namespace devices {
	struct _ScreenData {
		nsl::FrameBuffer fb;
		cv::Mat image;
	};

	inline bool Screen::Open() {
		_data = ngs::New(new _ScreenData());
		auto& data = *reinterpret_cast<_ScreenData*>(_data);

		if (!data.fb.Open("/dev/fb0")) {
			ngs::nos.Error("open frame buffer fail!\n");
			goto err_open;
		}

		return true;
	err_open:;

		ngs::Delete(&data);
		_data = nullptr;
		return false;
	}
	inline bool Screen::IsOpened()const { return _data; }
	inline void Screen::Close() {
		auto& data = *reinterpret_cast<_ScreenData*>(_data);
		data.fb.Close();
		ngs::Delete(&data);
		_data = nullptr;
	}

	inline void Screen::Update() {
		auto& data = *reinterpret_cast<_ScreenData*>(_data);
		size_t step = data.image.cols * 2;
		for (size_t y = 0; y < data.image.rows; y += 1) {
			std::memcpy(data.fb.GetData<ngs::byte>() + y * data.fb.GetWidth() * 2, data.image.data + y * step, step);
		}
	}

	inline void Screen::Show(const cv::Mat& image) {
		if (image.empty())return;
		auto& data = *reinterpret_cast<_ScreenData*>(_data);
		data.image = image;
		cv::cvtColor(data.image, data.image, cv::COLOR_BGR2BGR565);
	}
}