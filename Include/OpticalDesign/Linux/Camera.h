#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/DeviceConfig.h"
#include "OpticalDesign/Linux/NSLConfig.h"
#include "opencv2/opencv.hpp"

namespace devices {

	struct _CameraData {
		nsl::V4L2 device;
		std::vector<ngs::byte> data;
	};
	inline bool Camera::Open() {
		_data = ngs::New(new _CameraData());
		auto& data = *(_CameraData*)_data;

		if (!data.device.Open("/dev/video1") || !data.device.Initialize(640, 480, ngs::PixelFormat::MJPEG)) {
			ngs::nos.Error("camera initialize failed\n");
			goto err_camera;
		}
		ngs::nos.Log("Camera::Camera", "camera initialize successfully\n");

		return true;
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
	}
	inline bool Camera::IsOpened() const {
		if (!_data)return false;
		auto& data = *(_CameraData*)_data;
		return data.device.IsOpened();
	}
	inline std::vector<ngs::byte> Camera::Get() {
		auto& data = *(_CameraData*)_data;

		return data.data;
	}
};
