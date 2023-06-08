#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"
#include "OpticalDesign/Converter.h"
#include "OpticalDesign/DeviceAPI.h"

OPT_BEGIN

using namespace std::chrono_literals;

constexpr ngs::float32 SPEED = 0.3f;
class Foot {
public:
	enum class Mode {
		Idle,
		Rotating,
		Running,
	};
public:

	void Update(const Point& position, float angle) {
		_position = position;
		_angle = angle;

		if (_RequireRotate()) {
			_RotatePercent(ngs::Sign(_GetDeltaAngle()) * SPEED);
			return;
		}
		if (_RequireMove()) {
			_MovePercent(SPEED);
			return;
		}
		_mode = Mode::Idle;
		_linearVelocity = devices::g_engine->GetLinearVelocity();
	}

	void MoveTo(const Point& pos) {
		_aimPos = pos;
	}

	Mode GetMode()const { return _mode; }

	void Rotate90(int direct) {
		_RotatePercent(direct * 0.61f);
		std::this_thread::sleep_for(500ms);
		_RotatePercent(0);
	}
	void LittleRotate(int direct) {
		_RotatePercent(direct * 0.05f);
		std::this_thread::sleep_for(100ms);
		_RotatePercent(0);
	}
	void StopTime(int ms = 200) {
		_MovePercent(0);
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
	ngs::float32 GetLinearVelocity()const { return _linearVelocity; }
public:
	void _RotatePercent(ngs::float32 angularSpeed) {
		devices::g_engine->SetAngularVelocityPercent(angularSpeed);
		_mode = Mode::Rotating;
	}
	void _MovePercent(ngs::float32 speed) {
		devices::g_engine->SetLinearVelocityPercent(speed);
		_mode = Mode::Running;
	}
	Point _GetDistance()const { return _aimPos - _position; }
	float _GetDeltaAngle() const {
		float aim = _GetDistance().ArcTan();
		float cur = _angle;
		float delta = aim - cur;
		delta = fmod(delta, 2 * std::numbers::pi);
		if (delta > std::numbers::pi)
			delta -= 2 * std::numbers::pi;
		else if (delta < -std::numbers::pi)
			delta += 2 * std::numbers::pi;

		return delta;
	}
	bool _RequireRotate() const {
		return std::abs(_GetDeltaAngle()) > FOOT_ROTATE_ANGLE_THRESHOLD;
	}
	bool _RequireMove() const {
		return _GetDistance().Length() > FOOT_MOVE_DISTANCE_THRESHOLD;
	}
private:
	Point _position;
	float _angle;

	Point _aimPos;

	Mode _mode;
	ngs::float32 _linearVelocity = 0.0f;
};



OPT_END