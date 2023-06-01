#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/DeviceConfig.h"
#include "OpticalDesign/Linux/NSLConfig.h"
#include "opencv2/opencv.hpp"

namespace devices {

	struct _CameraData {
		nsl::V4L2 device;
		std::vector<ngs::byte> data;
#ifdef SHOW_ON_LCD
		nsl::FrameBuffer fb;
#endif
	};
	inline bool Camera::Open() {
		_data = ngs::New(new _CameraData());
		auto& data = *(_CameraData*)_data;

		if (!data.device.Open("/dev/video1") || !data.device.Initialize(320, 180, ngs::PixelFormat::MJPEG)) {
			ngs::nos.Error("camera initialize failed\n");
			goto err_camera;
		}
#ifdef SHOW_ON_LCD
		if (!data.fb.IsOpened() && !data.fb.Open("/dev/fb0")) {
			ngs::nos.Error("open fb fail!\n");
			goto err_fb;
		}
#endif
		ngs::nos.Log("Camera::Camera", "camera initialize successfully\n");

		return true;
#ifdef SHOW_ON_LCD
		err_fb : ;
		data.fb.Close();
#endif
	err_camera:;
		data.device.Close();

		ngs::Delete(&data);
		_data = nullptr;
		return false;
	}
	inline void Camera::Close() {
		auto& data = *(_CameraData*)_data;
		data.device.Close();

		ngs::Delete(&data);
		_data = nullptr;
	}
	inline void Camera::Update() {
		auto& data = *(_CameraData*)_data;

		data.device.Update();
		size_t size = data.device.GetFrameBufferSize();
		if (size > data.data.size())
			data.data.resize(size);
		if (!data.device.Read(data.data.data())) {
			ngs::nos.Error("read camera data fail!\n");
			return;
		}
#ifdef SHOW_ON_LCD
		cv::Mat image = cv::imdecode(data.data, cv::ImreadModes::IMREAD_COLOR);
		ngs::nos.Log("main", "start convert\n");
		for (size_t y = 0; y < image.rows; y += 1) {
			for (size_t x = 0; x < image.cols; x += 1) {
				auto argb24 = image.ptr<ngs::ARGB24>(y, x);
				data.fb.GetData<ngs::BGRA16>()[y * data.fb.GetWidth() + x] = ngs::BGRA16(argb24->StdBlue(), argb24->StdGreen(), argb24->StdRed(), argb24->StdAlpha());
			}
		}
#endif
	}
	inline bool Camera::IsOpened() const {
		if (!_data)return false;
		auto& data = *(_CameraData*)_data;
		return data.device.IsOpened();
	}
	inline const std::vector<ngs::byte>& Camera::Get() {
		auto& data = *(_CameraData*)_data;

		return data.data;
	}
};
