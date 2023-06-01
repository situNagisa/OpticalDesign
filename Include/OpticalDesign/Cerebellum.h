#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"
#include "OpticalDesign/Converter.h"
#include "OpticalDesign/DeviceAPI.h"

OPT_BEGIN

class Cerebellum {
public:

public:
	~Cerebellum() {
		if (devices::g_gyroscope->IsOpened()) {
			devices::g_gyroscope->Close();
		}
	}

	bool Initialize() {
		if (!devices::g_gyroscope->Open()) {
			return false;
		}
		return true;
	}
	void Update() {
		if (!devices::g_gyroscope->IsOpened() && !devices::g_gyroscope->Open()) {
			return;
		}
		devices::g_gyroscope->Update();
		_acceleration.Set(devices::g_gyroscope->GetAccelerationX(), devices::g_gyroscope->GetAccelerationY());
		_angularVelocity = devices::g_gyroscope->GetAngularVelocityZ();

	}

	Point GetAcceleration()const { return _acceleration; }
	ngs::float32 GetAngularVelocity()const { return _angularVelocity; }
private:


private:
	Point _acceleration = {};
	ngs::float32 _angularVelocity = 0.0f;
};

OPT_END