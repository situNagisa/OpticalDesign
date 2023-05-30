#pragma once

#include "OpticalDesign/Devices.h"
#include "OpticalDesign/Linux/Esp32.h"

namespace devices {

	inline bool Engine::Open() {
		if (g_esp32->IsOpened() || g_esp32->Open()) {
			ngs::nos.Log("Gyroscope::Gyroscope", "gyroscope initialize successfully\n");
			return true;
		}
		ngs::nos.Error("gyroscope initialize failed!\n");
		return false;
	}
	inline void Engine::Close() {
	}
	inline bool Engine::IsOpened() const {
		return g_esp32->IsOpened();
	}
	inline void Engine::Update() {
	}
	inline void Engine::SetLinearVelocityPercent(ngs::float32 percent) {
		g_esp32->SetLinearVelocityPercent(percent);
	}
	inline void Engine::SetAngularVelocityPercent(ngs::float32 percent) {
		g_esp32->SetAngularVelocityPercent(percent);
	}
}
