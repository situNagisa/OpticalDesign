#pragma once

#include "ESP32/Config.h"
#include "ESP32/PWM.h"

OPT_BEGIN

struct _Wheel {

	bool Open(
		ngs::pin_t gpio0, ngs::pin_t gpio1,
		ngs::pin_t impulse0, ngs::pin_t impulse1,
		ngs::pin_t pwm
	) {
		if (!_gpio0.IsOpened() && !_gpio0.Open(gpio0, ngs::GPIO::Mode::output))return false;
		if (!_gpio1.IsOpened() && !_gpio1.Open(gpio1, ngs::GPIO::Mode::output))return false;
		if (!_pwm.IsOpened() && !_pwm.Open(pwm))return false;
		if (!_impulse0.IsOpened() && !_impulse0.Open(impulse0, ngs::GPIO::Mode::input))return false;
		if (!_impulse1.IsOpened() && !_impulse1.Open(impulse1, ngs::GPIO::Mode::input))return false;

		_is_opened = true;
		ngs::nos.Log("_Wheel::Open", "gpio0:%d, gpio1:%d pwm:%d success!\n", gpio0, gpio1, pwm);
		return true;
	}
	void Update() {

	}

	bool IsOpened()const { return _is_opened; }
	void Close() {
		_gpio0.Close();
		_gpio1.Close();
		_pwm.Close();
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
		_pwm.Set(percent);
	}
	float GetLinearVelocity() {

	}
private:
	bool _is_opened = false;
	ngs::GPIO _gpio0, _gpio1;
	ngs::GPIO _impulse0, _impulse1;
	PWM _pwm;
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
		ngs::pin_t fl_gpio0, ngs::pin_t fl_gpio1, ngs::pin_t fl_pwm,
		ngs::pin_t bl_gpio0, ngs::pin_t bl_gpio1, ngs::pin_t bl_pwm,
		ngs::pin_t fr_gpio0, ngs::pin_t fr_gpio1, ngs::pin_t fr_pwm,
		ngs::pin_t br_gpio0, ngs::pin_t br_gpio1, ngs::pin_t br_pwm
	) {
		ngs::nos.Log("Motor::Open", "open motor\n");
		if (!_Left().IsOpened() && !_Left().Open(fl_gpio0, fl_gpio1, fl_pwm))return false;
		if (!_Right().IsOpened() && !_Right().Open(bl_gpio0, bl_gpio1, bl_pwm))return false;
		_is_opened = true;
		ngs::nos.Log("Motor::Open", "success!\n");
		return true;
	}
	bool IsOpened()const { return _is_opened; }
	void Close() {
		_Left().Close();
		_Right().Close();
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
		for (auto wheel : _wheels) {
			wheel.SetDirect(sgn);
			wheel.SetVelocityPercent(percent);
		}
	}
	void _SetAngularVelocityPercent(float percent) {
		int sgn = ngs::Sign(percent);
		percent *= sgn;
		_Left().SetDirect(sgn);
		_Left().SetDirect(-sgn);
		for (auto wheel : _wheels) {
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
