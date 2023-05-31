#include "ESP32/Rangefinder.h"

OPT_BEGIN

void UltrasonicRangefinder::Close() {
	_trig.Close();
	_echo.Close();
	_is_opened = false;
}

inline ngs::uint32 UltrasonicRangefinder::GetDistance()
{
	_trig.High();
	using std::chrono_literals::operator""ms;

	std::this_thread::sleep_for(1ms);
	_trig.Low();

	while (!_echo.Get());

	auto start = std::chrono::high_resolution_clock::now();
	while (_echo.Get());
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	return duration.count() / 60;
}

OPT_END
