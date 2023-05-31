#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/DeviceConfig.h"
#include "OpticalDesign/Linux/NSLConfig.h"

namespace devices {

	struct _CameraData {
		nsl::V4L2 device;
	};
	inline bool Camera::Open() {
		_data = ngs::New(new _CameraData());
		auto& data = *(_CameraData*)_data;

		data.device.Open("/dev/video1");

		if (!data.device.IsOpened()) {
			ngs::nos.Error("camera initialize failed\n");
			ngs::Delete(&data);
			_data = nullptr;
			return false;
		}
		data.device.Initialize(800, 480, ngs::PixelFormat::ARGB24, 30);
		ngs::nos.Log("Camera::Camera", "camera initialize successfully\n");

		return true;
	}
	inline void Camera::Close() {
		auto& data = *(_CameraData*)_data;
		data.device.Close();

		ngs::Delete(&data);
		_data = nullptr;
	}
	inline void Camera::Update() {

	}
	inline bool Camera::IsOpened() const {
		if (!_data)return false;
		auto& data = *(_CameraData*)_data;
		return data.device.IsOpened();
	}
	inline std::vector<ngs::byte> Camera::Get() {
		auto& data = *(_CameraData*)_data;

		return {};
	}
};
