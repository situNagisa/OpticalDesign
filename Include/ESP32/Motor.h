#pragma once

#include "ESP32/Config.h"
#include "ESP32/PWM.h"
#include "ESP32/PCNT.h"
#include "ESP32/Setting.h"

OPT_BEGIN

struct _Wheel {

	bool Open(
		ngs::pin_t gpio0, ngs::pin_t gpio1,
		ngs::pin_t impulse0, ngs::pin_t impulse1,
		ngs::pin_t pwm
	) {
		if (!_gpio0.IsOpened() && !_gpio0.Open(gpio0, ngs::GPIO::Mode::output))return false;
		if (!_gpio1.IsOpened() && !_gpio1.Open(gpio1, ngs::GPIO::Mode::output))return false;
		_gpio1.Low();
		if (!_pwm.IsOpened() && !_pwm.Open(pwm))return false;
		if (!_pcnt.IsOpened() && !_pcnt.Open(impulse0, impulse1, 100, -100))return false;

		_is_opened = true;
		ngs::nos.Log("_Wheel::Open", "gpio0:%d, gpio1:%d pwm:%d success!\n", gpio0, gpio1, pwm);

		_pid.SetPID(PID.x, PID.y, PID.z);
		return true;
	}
	void Update() {
		//_UpdatePCNT();
		//_aim_percent += _pid.GetOutput(_current_velocity, _aim_percent);
		_pwm.Set(_aim_percent);
	}

	bool IsOpened()const { return _is_opened; }
	void Close() {
		_gpio0.Close();
		_gpio1.Close();
		_pwm.Close();
		_pcnt.Close();
		_is_opened = false;
	}
	void SetDirect(int direct) {
		if (direct > 0) {
			_gpio0.Low();
			_gpio1.High();
		}
		else if (direct < 0) {
			_gpio0.High();
			_gpio1.Low();
		}
	}
	void SetVelocityPercent(float percent) {
		_aim_percent = percent;
	}
	float GetLinearVelocity() {
		return _current_velocity;
	}
private:
	void _UpdatePCNT() {
		auto count = _pcnt.GetPulseCount();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(_end - _now);

		_current_velocity = -((float)count / PLUSE_COUNT_PER_RAD) * WHEEL_CIRCUMFERENCE / duration.count() * 1'000'000;
		_end = _now;
		_now = std::chrono::high_resolution_clock::now();
		_pcnt.ClearPulseCount();
	}
public:
	std::chrono::high_resolution_clock::time_point _now = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point _end = std::chrono::high_resolution_clock::now();

	bool _is_opened = false;
	ngs::GPIO _gpio0, _gpio1;
	PCNT _pcnt;
	ngs::PID _pid;
	PWM _pwm;
	float _aim_percent = 0.0f;
	float _current_velocity = 0.0f;
};

class Motor {
private:
	NGS_TYPE_DEFINE(_Wheel, wheel);
public:
	enum class Mode {
		Linear,
		Angular
	};
public:

	bool Open(
		ngs::pin_t l_gpio0, ngs::pin_t l_gpio1, ngs::pin_t l_impulse0, ngs::pin_t l_impulse1, ngs::pin_t l_pwm,
		ngs::pin_t r_gpio0, ngs::pin_t r_gpio1, ngs::pin_t r_impulse0, ngs::pin_t r_impulse1, ngs::pin_t r_pwm
	) {
		ngs::nos.Log("Motor::Open", "open motor\n");
		if (!_Left().IsOpened() && !_Left().Open(l_gpio0, l_gpio1, l_impulse0, l_impulse1, l_pwm))return false;
		if (!_Right().IsOpened() && !_Right().Open(r_gpio0, r_gpio1, r_impulse0, r_impulse1, r_pwm))return false;
		_is_opened = true;
		ngs::nos.Log("Motor::Open", "success!\n");
		return true;
	}
	bool IsOpened()const { return _is_opened; }
	void Close() {
		for (auto& wheel : _wheels) {
			wheel.Close();
		}
		_is_opened = false;
	}
	void Update() {
		for (auto& wheel : _wheels) {
			wheel.Update();
		}
	}
	void SetMode(Mode mode) {
		_mode = mode;
	}
	void SetVelocityPercent(float percent) {
		if (_mode == Mode::Linear) {
			_SetLinearVelocityPercent(percent);
		}
		else {
			_SetAngularVelocityPercent(percent);
		}
	}
	float GetLinearVelocity() {
		return (_Left().GetLinearVelocity() + _Right().GetLinearVelocity()) / 2;
	}
private:
	void _SetLinearVelocityPercent(float percent) {
		int sgn = ngs::Sign(percent);
		percent *= sgn;
		for (auto& wheel : _wheels) {
			wheel.SetDirect(sgn);
			wheel.SetVelocityPercent(percent);
		}
	}
	void _SetAngularVelocityPercent(float percent) {
		int sgn = ngs::Sign(percent);
		percent *= sgn;
		_Left().SetDirect(sgn);
		_Right().SetDirect(-sgn);
		for (auto& wheel : _wheels) {
			wheel.SetVelocityPercent(percent);
		}
	}

	__wheel_ref _Left() { return _wheels[0]; }
	__wheel_ref _Right() { return _wheels[1]; }
private:
	bool _is_opened = false;
	Mode _mode = Mode::Linear;
	ngs::PID _pid;

	std::array<__wheel, 2> _wheels = {};
};

OPT_END
