#pragma once

#include "OpticalDesign/Devices.h"
namespace devices {

	inline bool Engine::Open() {
		return false;
	}
	inline void Engine::Close() {
	}
	inline bool Engine::IsOpened() const {
		return false;
	}
	inline void Engine::Update() {
	}
	inline void Engine::SetLinearVelocityPercent(ngs::float32 percent) {
	}
	inline void Engine::SetAngularVelocityPercent(ngs::float32 percent) {
	}
	inline ngs::float32 Engine::GetLinearVelocity()const {
		return 0.0f;
	}
}
