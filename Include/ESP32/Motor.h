#pragma once

#include "ESP32/Config.h"
#include "ESP32/PWM.h"
#include "ESP32/GPIO.h"

OPT_BEGIN

class Motor {
private:
	NGS_TYPE_DEFINE(PWM, pwm);
	struct _Wheel {

		bool Open(ngs::pin_t gpio0, ngs::pin_t gpio1, ngs::pin_t pwm) {
			if (!_gpio0.IsOpened() && !_gpio0.Open(gpio0, GPIO::Mode::Output))return false;
			if (!_gpio1.IsOpened() && !_gpio1.Open(gpio1, GPIO::Mode::Output))return false;
			if (!_pwm.IsOpened() && !_pwm.Open(pwm))return false;
			_is_opened = true;
			ngs::nos.Log("_Wheel::Open", "gpio0:%d, gpio1:%d pwm:%d success!\n", gpio0, gpio1, pwm);
			return true;
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
				_gpio0.Set(GPIO::Level::Low);
				_gpio1.Set(GPIO::Level::High);
			}
			else if (direct < 0) {
				_gpio0.Set(GPIO::Level::High);
				_gpio1.Set(GPIO::Level::Low);
			}
		}
		void SetVelocityPercent(float percent) {
			_pwm.Set(percent);
		}
	private:
		bool _is_opened = false;
		GPIO _gpio0, _gpio1;
		__pwm _pwm;
	};
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
		if (!_FL().IsOpened() && !_FL().Open(fl_gpio0, fl_gpio1, fl_pwm))return false;
		if (!_BL().IsOpened() && !_BL().Open(bl_gpio0, bl_gpio1, bl_pwm))return false;
		if (!_FR().IsOpened() && !_FR().Open(fr_gpio0, fr_gpio1, fr_pwm))return false;
		if (!_BR().IsOpened() && !_BR().Open(br_gpio0, br_gpio1, br_pwm))return false;
		_is_opened = true;
		ngs::nos.Log("Motor::Open", "success!\n");
		return true;
	}
	bool IsOpened()const { return _is_opened; }
	void Close() {
		_FL().Close();
		_BL().Close();
		_FR().Close();
		_BR().Close();
		_is_opened = false;
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
		_FL().SetDirect(sgn);
		_BL().SetDirect(sgn);
		_FR().SetDirect(-sgn);
		_BR().SetDirect(-sgn);
		for (auto wheel : _wheels) {
			wheel.SetVelocityPercent(percent);
		}
	}

	__wheel_ref _FL() { return _wheels[0]; }
	__wheel_ref _BL() { return _wheels[1]; }
	__wheel_ref _FR() { return _wheels[2]; }
	__wheel_ref _BR() { return _wheels[3]; }
private:
	bool _is_opened = false;
	Mode _mode = Mode::Linear;

	std::array<__wheel, 4> _wheels = {};
};

OPT_END
