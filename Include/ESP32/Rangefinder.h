#pragma once

#include "ESP32/Config.h"

OPT_BEGIN

class UltrasonicRangefinder {
public:

public:

	bool Open(ngs::pin_t trig, ngs::pin_t echo) {
		if (!_trig.IsOpened() && !_trig.Open(trig))return false;
		if (!_echo.IsOpened() && !_echo.Open(echo))return false;
		return true;
	}
	bool IsOpened()const { return _is_opened; }
	void Close();
	ngs::uint32 GetDistance();

private:

private:
	bool _is_opened = false;
	ngs::GPIO _trig, _echo;
};

OPT_END
