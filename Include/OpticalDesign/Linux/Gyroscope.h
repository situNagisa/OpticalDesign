#pragma once

#include "NGS/NGS.h"
#include "opencv2/opencv.hpp"
#include "OpticalDesign/DeviceConfig.h"
#include "OpticalDesign/Linux/NSLConfig.h"
#include "OpticalDesign/Linux/Esp32.h"

namespace devices {
	inline bool Gyroscope::Open() {
		if (g_esp32->IsOpened() || g_esp32->Open()) {
			ngs::nos.Log("Gyroscope::Gyroscope", "gyroscope initialize successfully\n");
			return true;
		}
		ngs::nos.Error("gyroscope initialize failed!\n");
		return false;
	}
	inline void Gyroscope::Close() {
	}
	inline bool Gyroscope::IsOpened() const {
		return g_esp32->IsOpened();
	}
	inline void Gyroscope::Update() {
		g_esp32->Update();
	}
	inline float Gyroscope::GetAccelerationX() const {
		return g_esp32->GetAcceleration().x;
	}
	inline float Gyroscope::GetAccelerationY() const {
		return g_esp32->GetAcceleration().y;
	}
	inline float Gyroscope::GetAccelerationZ() const {
		return g_esp32->GetAcceleration().z;
	}
	inline float Gyroscope::GetAngularVelocityX() const {
		return g_esp32->GetAngularVelocity().x;
	}
	inline float Gyroscope::GetAngularVelocityY() const {
		return g_esp32->GetAngularVelocity().y;
	}
	inline float Gyroscope::GetAngularVelocityZ() const {
		return g_esp32->GetAngularVelocity().z;
	}
	/*struct _GryoData {
		nsl::DeviceFile device;
	};
	inline bool Gyroscope::Open() {
		if (g_esp32->IsOpened() || g_esp32->Open()) {
			ngs::nos.Log("Gyroscope::Gyroscope", "gyroscope initialize successfully\n");
			return true;
		}
		ngs::nos.Error("gyroscope initialize failed!\n");
		return false;
	}
	inline void Gyroscope::Close() {
	}
	inline bool Gyroscope::IsOpened() const {
		return g_esp32->IsOpened();
	}
	inline void Gyroscope::Update() {

	}
	inline float Gyroscope::GetAccelerationX() const {
		return g_esp32->GetAcceleration().x;
	}
	inline float Gyroscope::GetAccelerationY() const {
		return g_esp32->GetAcceleration().y;
	}
	inline float Gyroscope::GetAccelerationZ() const {
		return g_esp32->GetAcceleration().z;
	}
	inline float Gyroscope::GetAngularVelocityX() const {
		return g_esp32->GetAngularVelocity().x;
	}
	inline float Gyroscope::GetAngularVelocityY() const {
		return g_esp32->GetAngularVelocity().y;
	}
	inline float Gyroscope::GetAngularVelocityZ() const {
		return g_esp32->GetAngularVelocity().z;
	}*/
};