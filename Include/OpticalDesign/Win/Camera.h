#pragma once

#include "NGS/NGS.h"
#include "opencv2/opencv.hpp"
#include "OpticalDesign/DeviceConfig.h"

namespace devices {

	struct _CameraData {
		cv::VideoCapture capture;
	};
	inline bool Camera::Open() {
		_data = ngs::New(new _CameraData());
		auto& data = *(_CameraData*)_data;

		data.capture.open(0);
		if (!data.capture.isOpened()) {
			ngs::nos.Error("打开摄像头失败");
			ngs::Delete(&data);
			_data = nullptr;
			return false;
		}
		ngs::nos.Log("Camera::Camera", "打开摄像头成功\n");
		return true;
	}
	inline void Camera::Close() {
		auto& data = *(_CameraData*)_data;
		data.capture.release();

		ngs::Delete(&data);
		_data = nullptr;
	}
	inline void Camera::Update() {

	}
	inline bool Camera::IsOpened() const {
		if (!_data)return false;
		auto& data = *(_CameraData*)_data;
		return data.capture.isOpened();
	}
	inline const std::vector<ngs::byte>& Camera::Get() {
		auto& data = *(_CameraData*)_data;
		cv::Mat buffer;

		data.capture >> buffer;
		if (buffer.empty())return {};
		std::vector<uchar> result;
		std::vector<int> param = { cv::IMWRITE_JPEG_QUALITY,95 };
		cv::imencode(".jpg", buffer, result, param);
		return result;
	}
};
